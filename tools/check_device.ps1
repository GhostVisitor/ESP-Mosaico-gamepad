# ============================================================================
#  check_device.ps1 - verify how Windows enumerated the CH552 gamepad
# ----------------------------------------------------------------------------
#  Usage:
#    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\check_device.ps1
#    powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\check_device.ps1 -VidPid "VID_1209&PID_0001"
#
#  Good result (HID gamepad):
#    - a node whose Class is HIDClass, and a "HID-compliant game controller"
#  Bad result (XInput hijack):
#    - a node with Class = XnaComposite  => that ID got claimed by the XInput
#      driver, which will NOT work with this HID-only firmware. Change VID/PID.
#
#  (ASCII only on purpose: Windows PowerShell 5.1 reads non-BOM files as ANSI)
# ============================================================================

param([string]$VidPid = 'VID_0079&PID_0006')

Write-Host "=== 1) nodes matching $VidPid ==="
$d = Get-PnpDevice -PresentOnly | Where-Object { $_.InstanceId -match $VidPid }
if (-not $d) {
  Write-Host "  (none) -> device not enumerated, or VID/PID in config.h differs" -ForegroundColor Yellow
} else {
  $d | Select-Object Status, Class, FriendlyName, InstanceId | Format-List
  foreach ($n in $d) {
    if ($n.Class -eq 'XnaComposite') {
      Write-Host "  [!] Class = XnaComposite => XInput driver owns it, this HID firmware will NOT work." -ForegroundColor Red
      Write-Host "      Use a neutral VID/PID (e.g. 0x0079/0x0006) instead." -ForegroundColor Red
    }
  }
}

Write-Host "=== 2) HID gamepad collections present (locale independent) ==="
$gc = @()
foreach ($h in (Get-PnpDevice -Class HIDClass -PresentOnly -ErrorAction SilentlyContinue)) {
  $hw = (Get-PnpDeviceProperty -InstanceId $h.InstanceId -KeyName 'DEVPKEY_Device_HardwareIds' -ErrorAction SilentlyContinue).Data
  if ($hw -match 'HID_DEVICE_UP:0001_U:0005') { $gc += $h }
}
if ($gc.Count -eq 0) {
  Write-Host "  (none) -> no HID game controller registered" -ForegroundColor Yellow
} else {
  $gc | Select-Object Status, Class, FriendlyName | Format-Table -AutoSize
  Write-Host "  => run 'joy.cpl' to test the buttons" -ForegroundColor Green
}
