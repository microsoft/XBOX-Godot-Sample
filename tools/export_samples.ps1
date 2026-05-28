<#
.SYNOPSIS
    Run Godot's headless export for one or more sample projects.

.DESCRIPTION
    For each requested sample, runs `godot --headless --import` (warm-up) then
    `godot --headless --export-{debug|release} <preset> <output>`. Outputs land
    under `sample\<name>\Build\` to match each sample's preconfigured
    `export_path` and the GDK packaging addon's `res://Build` convention.

    SCOPE: this is a RAW Godot export. It does NOT produce a GDK-packaged or
    sandbox-installable build. Packaging steps (MicrosoftGame.config copy,
    VC14 injection, executable rename, logo copy, addon DLL staging,
    `wdapp` registration, MSIXVC creation) are handled by the
    godot_gdk_packaging editor panel, not this script. Use this when you
    want a quick PC export and `tools\run_all_tests.ps1` -style automation;
    use the editor panel for full GDK packaging.

    The repository ships two samples under `sample\`:
      - `tutorial_app`        — integrated tutorial chain (T1-T8)
      - `tutorial_gameinput`  — standalone GameInput demo
    Pass either name (or both) via `-Sample`. The default `-Sample`
    value is empty so a no-arg invocation is a no-op; opt in
    explicitly per sample. Samples without an `export_presets.cfg`
    are reported and skipped.

.PARAMETER Sample
    One or more sample directory names under `sample\`. Default is empty
    (no samples). Pass `tutorial_app`, `tutorial_gameinput`, or both.

.PARAMETER Preset
    Godot export preset name. Default: `Windows Desktop`. Pass an alternate
    preset (e.g. `Xbox GDK (PC)`) to switch all selected samples to it; samples
    that don't define the preset are reported and skipped.

.PARAMETER Configuration
    `Debug` (default) -> `--export-debug`. `Release` -> `--export-release`.

.PARAMETER SkipImport
    Skip the per-sample `godot --headless --import` warm-up. Use when you know
    the project's `.godot/` cache is already populated from a recent run.

.PARAMETER Godot
    Explicit path to a Godot 4 executable. Otherwise the script searches:
      1. $env:GODOT_CONSOLE / $env:GODOT_BIN / $env:GODOT
      2. sample\Godot*_console.exe (highest version first)
      3. sample\Godot*.exe (highest version first)
      4. `godot` / `godot4` on PATH

.OUTPUTS
    Writes per-sample export bits under `sample\<name>\Build\`. Exits 0 when
    every requested sample exports successfully (skipped samples count as
    success unless explicitly requested), 1 otherwise.

.EXAMPLE
    .\tools\export_samples.ps1
    No-op (no samples requested). Returns exit code 0.

.EXAMPLE
    .\tools\export_samples.ps1 -Sample tutorial_app -Configuration Release
    Release export of `sample\tutorial_app\`.

.EXAMPLE
    .\tools\export_samples.ps1 -Sample tutorial_app,tutorial_gameinput
    Debug export of both samples in sequence.
#>
[CmdletBinding()]
param(
    [string[]]$Sample = @(),

    [string]$Preset = 'Windows Desktop',

    [ValidateSet('Debug', 'Release')]
    [string]$Configuration = 'Debug',

    [switch]$SkipImport,

    [string]$Godot = ''
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$script:RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# Mirrors tools\run_all_tests.ps1::Get-GodotExecutable; kept inline so this
# script has no run_all_tests.ps1 import dependency.
function Get-GodotExecutable {
    param([string]$Explicit)

    if (-not [string]::IsNullOrWhiteSpace($Explicit)) {
        if (Test-Path $Explicit) {
            return [System.IO.Path]::GetFullPath((Resolve-Path $Explicit).Path)
        }
        throw "Explicit -Godot path does not exist: $Explicit"
    }

    $candidates = [System.Collections.Generic.List[string]]::new()

    foreach ($envName in @('GODOT_CONSOLE', 'GODOT_BIN', 'GODOT')) {
        $value = [Environment]::GetEnvironmentVariable($envName)
        if (-not [string]::IsNullOrWhiteSpace($value)) { $candidates.Add($value) }
    }

    $sampleDir = Join-Path $script:RepoRoot 'sample'
    foreach ($pattern in @('Godot*_console.exe', 'Godot*.exe')) {
        Get-ChildItem -Path $sampleDir -Filter $pattern -File -ErrorAction SilentlyContinue |
            Sort-Object FullName -Descending |
            ForEach-Object { $candidates.Add($_.FullName) }
    }

    foreach ($commandName in @('godot', 'godot4')) {
        $cmd = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($null -ne $cmd -and -not [string]::IsNullOrWhiteSpace($cmd.Source)) {
            $candidates.Add($cmd.Source)
        }
    }

    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        if (Test-Path $candidate) {
            return [System.IO.Path]::GetFullPath((Resolve-Path $candidate).Path)
        }
    }

    throw "Could not find a Godot 4 executable. Set GODOT_CONSOLE / GODOT_BIN / GODOT, place a Godot console exe under sample\, or pass -Godot <path>."
}

# Section-aware INI walker. Returns an ordered list of preset entries:
#   [{ Name = <string>; Platform = <string>; ExportPath = <string>; }, ...]
# (export_presets.cfg uses Godot's INI dialect: `[preset.N]` headers,
# double-quoted values; we only read the fields we need.)
function Read-ExportPresets {
    param([string]$Path)

    if (-not (Test-Path $Path)) { return @() }

    $presets = New-Object System.Collections.Generic.List[hashtable]
    $current = $null

    foreach ($raw in Get-Content $Path) {
        $line = $raw.Trim()

        # Top-level preset header (`[preset.N]`) opens a new entry; any other
        # section header (`[preset.N.options]`, etc.) stops populating it.
        if ($line -match '^\[preset\.\d+\]$') {
            $current = @{ Name = ''; Platform = ''; ExportPath = '' }
            $presets.Add($current)
            continue
        }
        if ($line.StartsWith('[')) {
            $current = $null
            continue
        }

        if ($null -eq $current -or $line -eq '' -or $line.StartsWith(';')) { continue }

        if ($line -match '^(?<key>name|platform|export_path)="(?<val>.*)"\s*$') {
            switch ($Matches.key) {
                'name'        { $current.Name        = $Matches.val }
                'platform'    { $current.Platform    = $Matches.val }
                'export_path' { $current.ExportPath  = $Matches.val }
            }
        }
    }

    return $presets
}

function Invoke-Godot {
    param(
        [string]$Godot,
        [string[]]$Arguments,
        [string]$WorkingDirectory
    )

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $Godot
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true
    foreach ($a in $Arguments) { [void]$psi.ArgumentList.Add($a) }

    $proc = [System.Diagnostics.Process]::new()
    $proc.StartInfo = $psi

    # Async drain so a Godot stderr flood can't deadlock us. (Same contract as
    # tools\run_all_tests.ps1::Invoke-ChildProcess.)
    $stdoutBuf = [System.Text.StringBuilder]::new()
    $stderrBuf = [System.Text.StringBuilder]::new()

    $stdoutSub = Register-ObjectEvent -InputObject $proc -EventName OutputDataReceived `
        -MessageData $stdoutBuf -Action {
            if ($null -ne $EventArgs.Data) {
                [void]$Event.MessageData.AppendLine($EventArgs.Data)
                Write-Host $EventArgs.Data
            }
        }
    $stderrSub = Register-ObjectEvent -InputObject $proc -EventName ErrorDataReceived `
        -MessageData $stderrBuf -Action {
            if ($null -ne $EventArgs.Data) {
                [void]$Event.MessageData.AppendLine($EventArgs.Data)
                Write-Host $EventArgs.Data
            }
        }

    try {
        [void]$proc.Start()
        $proc.BeginOutputReadLine()
        $proc.BeginErrorReadLine()
        $proc.WaitForExit()
        return $proc.ExitCode
    } finally {
        Unregister-Event -SourceIdentifier $stdoutSub.Name -ErrorAction SilentlyContinue
        Unregister-Event -SourceIdentifier $stderrSub.Name -ErrorAction SilentlyContinue
        Remove-Job -Id $stdoutSub.Id -Force -ErrorAction SilentlyContinue
        Remove-Job -Id $stderrSub.Id -Force -ErrorAction SilentlyContinue
        $proc.Dispose()
    }
}

function Get-ProjectName {
    param([string]$ProjectGodotPath)

    if (-not (Test-Path $ProjectGodotPath)) { return 'GodotSample' }
    foreach ($line in Get-Content $ProjectGodotPath) {
        if ($line -match '^config/name\s*=\s*"(?<name>.*)"\s*$') { return $Matches.name }
    }
    return 'GodotSample'
}

# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

if ($Sample.Count -eq 0) {
    Write-Host "export_samples.ps1: no samples to export (sample/ is currently empty)."
    Write-Host "                   The tutorial-driven sample revamp (PR 3) will add"
    Write-Host "                   sample/tutorial_app/ and sample/tutorial_gameinput/."
    exit 0
}

$godotExe = Get-GodotExecutable -Explicit $Godot
Write-Host "export_samples.ps1: Godot=$godotExe Configuration=$Configuration Preset='$Preset'"
Write-Host "                   Samples=$($Sample -join ', ')"

$exportFlag = if ($Configuration -eq 'Release') { '--export-release' } else { '--export-debug' }
$results    = New-Object System.Collections.Generic.List[hashtable]
$explicitlyRequested = $PSBoundParameters.ContainsKey('Sample')

foreach ($name in $Sample) {
    $entry = @{ Sample = $name; Status = 'unknown'; Detail = '' }
    $sampleDir = Join-Path $script:RepoRoot "sample\$name"

    if (-not (Test-Path $sampleDir)) {
        $entry.Status = 'fail'
        $entry.Detail = "sample dir not found: $sampleDir"
        $results.Add($entry)
        continue
    }

    $presetsPath = Join-Path $sampleDir 'export_presets.cfg'
    if (-not (Test-Path $presetsPath)) {
        if ($explicitlyRequested) {
            $entry.Status = 'fail'
            $entry.Detail = "no export_presets.cfg (this sample is intentionally headless-only; see sample\$name\README.md)"
        } else {
            $entry.Status = 'skip'
            $entry.Detail = 'no export_presets.cfg (intentionally headless-only)'
        }
        $results.Add($entry)
        continue
    }

    $presets = Read-ExportPresets -Path $presetsPath
    $match = $presets | Where-Object { $_.Name -eq $Preset } | Select-Object -First 1
    if ($null -eq $match) {
        $available = ($presets | ForEach-Object { $_.Name }) -join ', '
        $entry.Status = 'skip'
        $entry.Detail = "preset '$Preset' not in $name (available: $available)"
        $results.Add($entry)
        continue
    }

    $exportPathRel = $match.ExportPath
    if ([string]::IsNullOrWhiteSpace($exportPathRel)) {
        $projectName = Get-ProjectName -ProjectGodotPath (Join-Path $sampleDir 'project.godot')
        $exportPathRel = "Build/$projectName.exe"
        Write-Host "  [$name] preset '$Preset' has empty export_path; synthesising '$exportPathRel'"
    }

    $exportPathAbs = Join-Path $sampleDir $exportPathRel
    $outputDir     = Split-Path -Parent $exportPathAbs
    if (-not (Test-Path $outputDir)) { [void](New-Item -ItemType Directory -Path $outputDir -Force) }

    if (-not $SkipImport) {
        Write-Host "  [$name] importing ($godotExe --headless --import)"
        $importExit = Invoke-Godot -Godot $godotExe -WorkingDirectory $sampleDir `
            -Arguments @('--headless', '--import')
        if ($importExit -ne 0) {
            $entry.Status = 'fail'
            $entry.Detail = "import exited $importExit"
            $results.Add($entry)
            continue
        }
    }

    Write-Host "  [$name] exporting ($exportFlag '$Preset' -> $exportPathRel)"
    $exportExit = Invoke-Godot -Godot $godotExe -WorkingDirectory $sampleDir `
        -Arguments @('--headless', $exportFlag, $Preset, $exportPathAbs)
    if ($exportExit -ne 0 -or -not (Test-Path $exportPathAbs)) {
        $entry.Status = 'fail'
        $entry.Detail = "export exited $exportExit; output exists=$([bool](Test-Path $exportPathAbs))"
        $results.Add($entry)
        continue
    }

    $entry.Status = 'pass'
    $entry.Detail = $exportPathAbs
    $results.Add($entry)
}

Write-Host ''
Write-Host '== Summary =='
foreach ($r in $results) {
    $tag = switch ($r.Status) {
        'pass' { 'PASS' }
        'skip' { 'SKIP' }
        'fail' { 'FAIL' }
        default { 'UNK ' }
    }
    Write-Host ("  {0}: {1}  {2}" -f $tag, $r.Sample, $r.Detail)
}

$failed = @($results | Where-Object { $_.Status -eq 'fail' })
if ($failed.Count -gt 0) {
    Write-Host ''
    Write-Host "export_samples.ps1: FAIL ($($failed.Count) sample(s) failed)" -ForegroundColor Red
    exit 1
}

Write-Host ''
Write-Host 'export_samples.ps1: OK' -ForegroundColor Green
exit 0
