param(
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"

$root = $PSScriptRoot
$packRoot = Join-Path $root "resource_pack"
if (-not (Test-Path $packRoot)) {
    throw "Resource-pack source folder not found: $packRoot"
}

if ([string]::IsNullOrWhiteSpace($OutputPath)) {
    $OutputPath = Join-Path $root "content\launch_point_dlc.zip"
}

$outputDirectory = Split-Path -Parent $OutputPath
New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null

if (Test-Path $OutputPath) {
    Remove-Item -Force $OutputPath
}

Push-Location $packRoot
try {
    Compress-Archive -Path * -DestinationPath $OutputPath -CompressionLevel Optimal
} finally {
    Pop-Location
}

Write-Host "Wrote DLC resource pack: $OutputPath"
Write-Host "Use package-relative path: content/launch_point_dlc.zip"
