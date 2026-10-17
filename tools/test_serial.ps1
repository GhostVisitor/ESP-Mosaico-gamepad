# ============================================================================
#  test_serial.ps1 - capture + decode the CH552 UART frames (P3.1, 19200 8N1)
# ----------------------------------------------------------------------------
#  Frame: A5 5A LO HI XOR      (LO/HI = 12-bit key mask)
#
#  Design notes (why this version):
#    - the receive loop does NOT print anything, so it never falls behind and
#      never loses a byte; every byte is timestamped on arrival
#    - after the capture, all frames and their intervals are reported, so we can
#      tell "heartbeat" frames (fixed period) from "key press" frames
#
#  Usage:
#    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\test_serial.ps1 -Port COM3
#    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\test_serial.ps1 -Port COM3 -Seconds 20 -Echo
#
#  Wiring:  CH552 P3.1 (TXD) -> adapter RXD,  GND -> GND,  adapter TXD not used
#
#  (ASCII only on purpose: Windows PowerShell 5.1 reads non-BOM files as ANSI)
# ============================================================================

param(
  [Parameter(Mandatory = $true)][string]$Port,
  [int]$Baud = 19200,
  [int]$Seconds = 20,
  [switch]$Echo              # print every frame, not only the decoded changes
)

$ErrorActionPreference = 'Stop'
$KEY_NAMES = @('UP','LEFT','DOWN','RIGHT','Y','X','A','B','SELECT','START','LT','RT')

# ---------------------------------------------------------------- open port
$sp = New-Object System.IO.Ports.SerialPort $Port, $Baud, 'None', 8, 'One'
$sp.ReadTimeout = 200
$sp.Open()
Write-Host "[serial] $Port @ $Baud 8N1 opened, capturing $Seconds s ..." -ForegroundColor Green
Write-Host "         (receive loop prints nothing, so no frame can be lost)" -ForegroundColor DarkGray

# --------------------------------------------------- capture with timestamps
$bytes = New-Object System.Collections.Generic.List[byte]
$times = New-Object System.Collections.Generic.List[datetime]
$buf   = New-Object byte[] 4096
$deadline = (Get-Date).AddSeconds($Seconds)

try {
  while ((Get-Date) -lt $deadline) {
    $n = $sp.BytesToRead
    if ($n -gt 0) {
      if ($n -gt $buf.Length) { $n = $buf.Length }
      $stamp = Get-Date
      $k = $sp.Read($buf, 0, $n)
      for ($i = 0; $i -lt $k; $i++) {
        $bytes.Add($buf[$i])
        $times.Add($stamp)
      }
    } else {
      Start-Sleep -Milliseconds 1
    }
  }
}
finally {
  $sp.Close()
}

Write-Host ("[serial] captured {0} bytes in {1:N2} s" -f $bytes.Count, $Seconds) -ForegroundColor Cyan

# ------------------------------------------------------------ parse frames
$frames = New-Object System.Collections.ArrayList
$i = 0
$bad = 0
while ($i -lt ($bytes.Count - 4)) {
  if ($bytes[$i] -ne 0xA5 -or $bytes[$i + 1] -ne 0x5A) { $i++; continue }
  $lo = $bytes[$i + 2]; $hi = $bytes[$i + 3]; $chk = $bytes[$i + 4]
  $calc = (0xA5 -bxor 0x5A -bxor $lo -bxor $hi)
  if ($chk -ne $calc) { $bad++; $i++; continue }
  # NOTE: PowerShell's -shl / -bor keep the type of the LEFT operand.
  #       $hi comes from a [byte] array, so "[byte]1 -shl 8" wraps to 0 and
  #       silently kills bits 8..15!  Always cast to [int] first.
  $mask = [int]$lo -bor ([int]$hi -shl 8)
  $names = @()
  for ($b = 0; $b -lt 12; $b++) { if ($mask -band (1 -shl $b)) { $names += $KEY_NAMES[$b] } }
  [void]$frames.Add([pscustomobject]@{
    T    = $times[$i]
    Raw  = ('A5 5A {0:X2} {1:X2} {2:X2}' -f $lo, $hi, $chk)
    Mask = $mask
    Keys = if ($names.Count) { $names -join '+' } else { '(none)' }
  })
  $i += 5
}

Write-Host ("[serial] valid frames: {0}   checksum errors: {1}" -f $frames.Count, $bad) -ForegroundColor Cyan
if ($frames.Count -eq 0) { Write-Host "[!] no valid frame - wrong baud? wrong wiring?" -ForegroundColor Yellow; exit 1 }

# ------------------------------------------------- intervals / classification
$dts = @()
for ($f = 1; $f -lt $frames.Count; $f++) { $dts += ($frames[$f].T - $frames[$f-1].T).TotalMilliseconds }
$sorted = @($dts | Sort-Object)
$median = if ($sorted.Count) { $sorted[[int]($sorted.Count / 2)] } else { 0 }
$avg = if ($dts.Count) { ($dts | Measure-Object -Average).Average } else { 0 }

Write-Host ("[serial] frame interval: median {0:N0} ms, average {1:N0} ms, min {2:N0} ms, max {3:N0} ms" -f `
  $median, $avg, ($sorted | Select-Object -First 1), ($sorted | Select-Object -Last 1)) -ForegroundColor Cyan
Write-Host ("[serial] => heartbeat period is about {0:N0} ms; frames much closer than that = key events" -f $median) -ForegroundColor Cyan
Write-Host ""

# ------------------------------------------------------------------- report
$lastMask = -1
for ($f = 0; $f -lt $frames.Count; $f++) {
  $fr = $frames[$f]
  $dt = if ($f -eq 0) { 0 } else { ($fr.T - $frames[$f-1].T).TotalMilliseconds }
  $show = $Echo -or ($fr.Mask -ne $lastMask) -or ($dt -lt ($median * 0.6))
  if ($show) {
    $flag = if ($dt -lt ($median * 0.6) -and $f -gt 0) { ' *off-beat*' } else { '' }
    Write-Host ("{0:HH:mm:ss.fff}  +{1,5:N0}ms  {2}  mask=0x{3:X4}  {4}{5}" -f `
      $fr.T, $dt, $fr.Raw, $fr.Mask, $fr.Keys, $flag)
  }
  $lastMask = $fr.Mask
}

# ------------------------------------------------------- distinct masks seen
Write-Host ""
$distinct = $frames | Group-Object Mask | Sort-Object Name
Write-Host "[serial] distinct masks seen:" -ForegroundColor Cyan
foreach ($g in $distinct) {
  $m = [int]$g.Name
  $nm = @()
  for ($b = 0; $b -lt 12; $b++) { if ($m -band (1 -shl $b)) { $nm += $KEY_NAMES[$b] } }
  Write-Host ("   0x{0:X4}  x{1,-4}  {2}" -f $m, $g.Count, $(if ($nm.Count) { $nm -join '+' } else { '(none)' }))
}
