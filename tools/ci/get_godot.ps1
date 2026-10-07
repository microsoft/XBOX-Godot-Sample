#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Acquire a pinned, SHA-512-verified stock Godot build.

.DESCRIPTION
    Thin CLI over tools\ci\GodotAcquisition.psm1 so CI and local tooling share
    one implementation of the pin-verify-extract policy:

      - .github\actions\setup-godot calls it (Pin -> cache -> Install -> Resolve)
      - tools\validate_pr_local.ps1 calls it (Install, into run-private dirs)

    Modes:
      Pin      Print the pinned lowercase SHA-512 for -Version. Fails when the
               version is not pinned in the manifest. Used for cache keys, and
               to reject unpinned versions even on a cache hit.
      Install  Download (staging dir outside the destination), verify SHA-512,
               extract into <DestinationRoot>\<Version>, resolve the executable
               and confirm `--version`. No-ops when already extracted unless
               -Force. Emits JSON.
      Resolve  Resolve the executable under <DestinationRoot>\<Version> and
               confirm `--version`, without downloading. Emits JSON.

.PARAMETER Version
    A godotengine/godot-builds release tag, e.g. '4.7.1-stable'. When omitted,
    the manifest's 'default' is used.

.PARAMETER ManifestPath
    Path to godot-versions.json. Defaults to .github\godot-versions.json
    relative to this script's repository root.

.PARAMETER DestinationRoot
    Directory that will contain <Version>\. Required for Install and Resolve.

.PARAMETER Flavor
    'console' (default, required for captured stdout on Windows) or 'editor'.

.PARAMETER ArchiveCacheDir
    Optional directory for verified archives. Cached archives are re-verified
    on every use and discarded when they do not match the pin.

.EXAMPLE
    pwsh -File tools\ci\get_godot.ps1 -Mode Pin -Version 4.7.1-stable

.EXAMPLE
    pwsh -File tools\ci\get_godot.ps1 -Mode Install -Version 4.6.1-stable `
        -DestinationRoot D:\a\_temp\godot
#>
[CmdletBinding()]
param(
    [ValidateSet('Pin', 'Install', 'Resolve')][string]$Mode = 'Install',
    [string]$Version,
    [string]$ManifestPath,
    [string]$DestinationRoot,
    [ValidateSet('console', 'editor')][string]$Flavor = 'console',
    [string]$ArchiveCacheDir,
    [switch]$Force
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Import-Module (Join-Path $PSScriptRoot 'GodotAcquisition.psm1') -Force

if (-not $ManifestPath) {
    $repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
    $ManifestPath = Join-Path $repoRoot '.github\godot-versions.json'
}
$ManifestPath = [System.IO.Path]::GetFullPath($ManifestPath)

if (-not $Version) {
    $Version = (Read-GodotManifest -ManifestPath $ManifestPath).Default
}

switch ($Mode) {
    'Pin' {
        (Get-GodotPin -Version $Version -ManifestPath $ManifestPath -Flavor $Flavor).Sha512
    }
    'Install' {
        if (-not $DestinationRoot) { throw "-DestinationRoot is required for -Mode Install." }
        $result = Install-Godot -Version $Version -ManifestPath $ManifestPath `
            -DestinationRoot ([System.IO.Path]::GetFullPath($DestinationRoot)) `
            -Flavor $Flavor -ArchiveCacheDir $ArchiveCacheDir -Force:$Force
        $result | ConvertTo-Json -Depth 4 -Compress
    }
    'Resolve' {
        if (-not $DestinationRoot) { throw "-DestinationRoot is required for -Mode Resolve." }
        $root = [System.IO.Path]::GetFullPath($DestinationRoot)
        # Resolve-only: the archive must already be present (cache hit). The
        # pin lookup still runs so an unpinned version is refused even when the
        # bits were restored from a cache.
        $pin = Get-GodotPin -Version $Version -ManifestPath $ManifestPath -Flavor $Flavor
        $engineDir = Join-Path $root $Version
        $exe = Resolve-GodotExecutable -Root $engineDir -Version $Version -Flavor $Flavor
        if (-not $exe) {
            throw "Godot executable '$($pin.ExecutableName)' not found under '$engineDir'."
        }
        $reported = (& $exe --version 2>&1 | Out-String).Trim()
        if (-not (Test-GodotReportedVersion -Version $Version -Reported $reported)) {
            throw "Engine at '$exe' reports version '$reported', which does not identify '$Version'."
        }
        [pscustomobject]@{
            Version         = $Version
            Flavor          = $Flavor
            Path            = $exe
            Sha512          = $pin.Sha512
            EngineDir       = $engineDir
            Url             = $pin.Url
            FromCache       = $true
            Downloaded      = $false
            ReportedVersion = $reported
        } | ConvertTo-Json -Depth 4 -Compress
    }
}
