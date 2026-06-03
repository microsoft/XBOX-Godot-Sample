# Xbox Live Player Data Reset
# Resets achievements, stats, and leaderboards for a test account.
# Requires: GDK installed, Partner Center "Tools Access" permission.

param(
    [string]$ConfigPath = "$PSScriptRoot\..\sample\sample_config.cfg"
)

$ErrorActionPreference = "Stop"

Write-Host "=== Xbox Live Player Data Reset ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Resets achievements, stats, and leaderboards for a test account."
Write-Host ""

$GdkBin = "C:\Program Files (x86)\Microsoft GDK\bin"
$ResetExe = Join-Path $GdkBin "XblPlayerDataReset.exe"
$DevAccountExe = Join-Path $GdkBin "XblDevAccount.exe"

if (-not (Test-Path $ResetExe)) {
    Write-Host "ERROR: XblPlayerDataReset.exe not found at: $ResetExe" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

if (-not (Test-Path $DevAccountExe)) {
    Write-Host "ERROR: XblDevAccount.exe not found at: $DevAccountExe" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "Found tools in: $GdkBin" -ForegroundColor Green
Write-Host ""

# Step 1: Sign in to Partner Center
Write-Host "Step 1: Sign in to Partner Center" -ForegroundColor Yellow
Write-Host "-----------------------------------"
& $DevAccountExe signin
if ($LASTEXITCODE -ne 0) {
    Write-Host "ERROR: Partner Center sign-in failed." -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}
Write-Host ""

# Step 2: Collect parameters
# Try to read SCID from the chosen sample config
$DefaultScid = ""
$SampleConfig = $ConfigPath
if (Test-Path $SampleConfig) {
    $match = Select-String -Path $SampleConfig -Pattern 'scid\s*=\s*"([^"]+)"'
    if ($match) { $DefaultScid = $match.Matches[0].Groups[1].Value }
}

if ($DefaultScid) {
    $Scid = Read-Host "Service Config ID (SCID) [$DefaultScid]"
    if ([string]::IsNullOrWhiteSpace($Scid)) { $Scid = $DefaultScid }
} else {
    $Scid = Read-Host "Service Config ID (SCID)"
}

$Sandbox = Read-Host "Sandbox ID"
$Xuid = Read-Host "Player XUID to reset"

Write-Host ""
Write-Host "Step 2: Resetting player data..." -ForegroundColor Yellow
Write-Host "  SCID:    $Scid"
Write-Host "  Sandbox: $Sandbox"
Write-Host "  XUID:    $Xuid"
Write-Host ""

& $ResetExe --scid $Scid --sandbox $Sandbox --xuid $Xuid

Write-Host ""
Write-Host "Step 3: Signing out of Partner Center" -ForegroundColor Yellow
& $DevAccountExe signout 2>$null

Write-Host ""
Write-Host "Done. Restart the game to re-test achievements." -ForegroundColor Green
Read-Host "Press Enter to exit"
