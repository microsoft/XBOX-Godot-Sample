<#
.SYNOPSIS
    Configure (when needed) and build the XBOX Godot Sample addon DLLs.

.DESCRIPTION
    Convenience wrapper around the canonical two-step build:

        cmake --preset <configure-preset>
        cmake --build --preset <build-preset>

    Selects the right configure-preset / build-preset / binary-dir trio from a
    single `-Preset` argument, runs `cmake --preset` only when the binary dir
    is missing or `-Reconfigure` / `-Clean` is set, then invokes the matching
    build preset.

    Raw cmake commands stay supported; this script just removes the manual
    step. See docs\getting-started.md for the canonical commands.

.PARAMETER Preset
    Configure preset:
      - `default`        (all addons; binary dir `build`)
      - `gdk-only`       (godot_gdk only; binary dir `build/gdk-only`)
      - `playfab-only`   (godot_playfab only; binary dir `build/playfab-only`)
      - `gameinput-only` (godot_gameinput only; binary dir `build/gameinput-only`)
      - `addon-package`  (all addons for drop-in zip staging; binary dir `build/addon-package`)

.PARAMETER Configuration
    `Debug` (default) or `Release`. Maps to the matching build preset for the
    selected configure preset.

.PARAMETER Clean
    `Remove-Item -Recurse -Force` the binary dir before configure. Implies
    `-Reconfigure`.

.PARAMETER Reconfigure
    Force `cmake --preset <configure-preset>` even when the binary dir's
    `CMakeCache.txt` already exists.

.OUTPUTS
    Exits 0 on success; otherwise the non-zero exit code from cmake.

.EXAMPLE
    .\tools\build_addons.ps1
    Debug build of every addon (the most common case).

.EXAMPLE
    .\tools\build_addons.ps1 -Configuration Release
    Release build of every addon.

.EXAMPLE
    .\tools\build_addons.ps1 -Preset gdk-only -Clean
    Wipe `build/gdk-only`, reconfigure, then build only godot_gdk in Debug.
#>
[CmdletBinding()]
param(
    [ValidateSet('default', 'gdk-only', 'playfab-only', 'gameinput-only', 'addon-package')]
    [string]$Preset = 'default',

    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',

    [switch]$Clean,

    [switch]$Reconfigure
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$script:RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# Configure preset -> (build-preset prefix, binary dir relative to repo root).
# Mirrors CMakePresets.json. Update both files together if presets change.
$script:PresetMap = @{
    'default'        = @{ BuildPrefix = '';            BinaryDir = 'build' }
    'gdk-only'       = @{ BuildPrefix = '-gdk';        BinaryDir = 'build/gdk-only' }
    'playfab-only'   = @{ BuildPrefix = '-playfab';    BinaryDir = 'build/playfab-only' }
    'gameinput-only' = @{ BuildPrefix = '-gameinput';  BinaryDir = 'build/gameinput-only' }
    'addon-package'  = @{ BuildPrefix = '-addon-package'; BinaryDir = 'build/addon-package' }
}

function Invoke-Cmake {
    param([string[]]$Arguments)

    Push-Location $script:RepoRoot
    try {
        & cmake @Arguments
        $exit = $LASTEXITCODE
        if ($exit -ne 0) {
            throw "cmake $($Arguments -join ' ') exited with code $exit"
        }
    } finally {
        Pop-Location
    }
}

$entry = $script:PresetMap[$Preset]
$binaryDirAbs = Join-Path $script:RepoRoot $entry.BinaryDir
$buildPreset  = $Configuration.ToLowerInvariant() + $entry.BuildPrefix

Write-Host "build_addons.ps1: Preset=$Preset Configuration=$Configuration BuildPreset=$buildPreset BinaryDir=$($entry.BinaryDir)"

if ($Clean -and (Test-Path $binaryDirAbs)) {
    Write-Host "  Cleaning $binaryDirAbs"
    Remove-Item -Recurse -Force $binaryDirAbs
}

$cacheFile = Join-Path $binaryDirAbs 'CMakeCache.txt'
$needConfigure = $Clean.IsPresent -or $Reconfigure.IsPresent -or -not (Test-Path $cacheFile)

if ($needConfigure) {
    Write-Host "  Configuring (cmake --preset $Preset)"
    Invoke-Cmake @('--preset', $Preset)
} else {
    Write-Host "  Reusing existing $($entry.BinaryDir) (use -Reconfigure to force)"
}

Write-Host "  Building (cmake --build --preset $buildPreset)"
Invoke-Cmake @('--build', '--preset', $buildPreset)

Write-Host "build_addons.ps1: OK"
