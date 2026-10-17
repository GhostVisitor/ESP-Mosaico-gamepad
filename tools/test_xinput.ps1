# ============================================================================
#  test_xinput.ps1 - read gamepad state via XInput API
# ----------------------------------------------------------------------------
#  Usage:
#    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\test_xinput.ps1
#    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\test_xinput.ps1 -Seconds 15
#  (ASCII only on purpose: Windows PowerShell 5.1 reads non-BOM files as ANSI)
# ============================================================================

param([int]$Seconds = 0)   # 0 = run forever, Ctrl+C to quit

$code = @'
using System;
using System.Runtime.InteropServices;
public static class XI {
  [StructLayout(LayoutKind.Sequential)]
  public struct XINPUT_GAMEPAD {
    public ushort wButtons; public byte bLeftTrigger; public byte bRightTrigger;
    public short sThumbLX; public short sThumbLY; public short sThumbRX; public short sThumbRY;
  }
  [StructLayout(LayoutKind.Sequential)]
  public struct XINPUT_STATE { public uint dwPacketNumber; public XINPUT_GAMEPAD Gamepad; }
  [DllImport("xinput1_4.dll", EntryPoint="XInputGetState")]
  public static extern uint XInputGetState14(uint i, ref XINPUT_STATE s);
  [DllImport("xinput1_3.dll", EntryPoint="XInputGetState")]
  public static extern uint XInputGetState13(uint i, ref XINPUT_STATE s);
}
'@
Add-Type -TypeDefinition $code -ErrorAction SilentlyContinue

function Get-XInputState([uint32]$slot, [ref]$state) {
  try   { return [XI]::XInputGetState14($slot, $state) }
  catch { return [XI]::XInputGetState13($slot, $state) }
}

$BTN = @{
  0x1000 = 'A'; 0x2000 = 'B'; 0x4000 = 'X'; 0x8000 = 'Y';
  0x0100 = 'LB'; 0x0200 = 'RB';
  0x0020 = 'Back'; 0x0010 = 'Start';
  0x0040 = 'L3'; 0x0080 = 'R3';
  0x0001 = 'Up'; 0x0002 = 'Down'; 0x0004 = 'Left'; 0x0008 = 'Right'
}

$state = New-Object XI+XINPUT_STATE
$found = -1
for ($i = 0; $i -lt 4; $i++) {
  if ((Get-XInputState $i ([ref]$state)) -eq 0) { $found = $i; break }
}

if ($found -lt 0) {
  Write-Host "[XInput] no pad in slots 0..3 (ret=1163 ERROR_DEVICE_NOT_CONNECTED)" -ForegroundColor Yellow
  Write-Host "        => Windows did NOT expose it as a usable XInput device" -ForegroundColor Yellow
  exit 1
}

Write-Host "[XInput] pad found in slot $found. Press buttons and watch the changes." -ForegroundColor Green
Write-Host "pkt        buttons  LT   RT   LX     LY     RX     RY     pressed"

$lastPkt = -1
$deadline = if ($Seconds -gt 0) { (Get-Date).AddSeconds($Seconds) } else { [datetime]::MaxValue }

while ((Get-Date) -lt $deadline) {
  if ((Get-XInputState $found ([ref]$state)) -ne 0) { Write-Host "device disconnected"; break }
  $g = $state.Gamepad
  if ($state.dwPacketNumber -ne $lastPkt) {
    $lastPkt = $state.dwPacketNumber
    $names = @()
    foreach ($k in $BTN.Keys) { if ($g.wButtons -band $k) { $names += $BTN[$k] } }
    $pressed = if ($names.Count) { $names -join '+' } else { '-' }
    $line = "{0,-10} 0x{1:X4}   {2,-4} {3,-4} {4,-6} {5,-6} {6,-6} {7,-6} {8}" -f $state.dwPacketNumber, $g.wButtons, $g.bLeftTrigger, $g.bRightTrigger, $g.sThumbLX, $g.sThumbLY, $g.sThumbRX, $g.sThumbRY, $pressed
    Write-Host $line
  }
  Start-Sleep -Milliseconds 20
}
