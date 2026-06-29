<#
.SYNOPSIS
    Regression-replays the libFuzzer fuzz targets over their committed corpora.

.DESCRIPTION
    This is the PR-gate entry point for fuzzing. It does NOT actively fuzz
    (no mutation, no time budget). For each built fuzz executable it replays the
    committed corpus directory once -- libFuzzer runs every corpus file as a
    single input and exits non-zero on the first crash, leak, or timeout. This
    makes the gate deterministic: a green run means every committed regression
    input still passes; a red run means a known-bad input reproduces.

    Fuzz executables are expected under <BuildDir>\bin\<Config>\<target>.exe
    (the layout produced by `cmake --build build/fuzz --preset debug-fuzz`).
    Each target's corpus lives at <CorpusRoot>\<target>\. A target with an empty
    or missing corpus is still smoke-run with `-runs=0` so a link/load/ASan-init
    failure is caught.

    Build the targets first:
        cmake --preset fuzz
        cmake --build build/fuzz --preset debug-fuzz

.PARAMETER BuildDir
    Fuzz build directory. Default: build\fuzz.

.PARAMETER CorpusRoot
    Root directory holding per-target corpus subdirectories.
    Default: tests\cpp\fuzz\corpus.

.PARAMETER Config
    Build configuration subfolder under <BuildDir>\bin. Default: Debug.

.PARAMETER Targets
    Optional list of fuzz target base names to restrict the run to. Default: all
    discovered fuzz executables.

.PARAMETER TimeoutSec
    Per-input libFuzzer timeout (-timeout). Default: 25.

.OUTPUTS
    Exits 0 if every replayed target passed, 1 otherwise.
#>
[CmdletBinding()]
param(
    [string]$BuildDir = 'build/fuzz',
    [string]$CorpusRoot = 'tests/cpp/fuzz/corpus',
    [string]$Config = 'Debug',
    [string[]]$Targets,
    [int]$TimeoutSec = 25
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$repoRoot = Split-Path -Parent $PSScriptRoot
$binDir = Join-Path $repoRoot (Join-Path $BuildDir (Join-Path 'bin' $Config))
$corpusBase = Join-Path $repoRoot $CorpusRoot

if (-not (Test-Path $binDir)) {
    Write-Error "Fuzz binary directory not found: $binDir. Build first: cmake --preset fuzz; cmake --build $BuildDir --preset debug-fuzz"
    exit 1
}

$exes = Get-ChildItem -Path $binDir -Filter '*.exe' -File -ErrorAction SilentlyContinue
if ($Targets) {
    $exes = $exes | Where-Object { $Targets -contains $_.BaseName }
}

if (-not $exes -or $exes.Count -eq 0) {
    Write-Error "No fuzz executables found under $binDir (filter: $($Targets -join ', '))."
    exit 1
}

$results = [System.Collections.Generic.List[object]]::new()
$anyFailed = $false

foreach ($exe in ($exes | Sort-Object Name)) {
    $name = $exe.BaseName
    $corpusDir = Join-Path $corpusBase $name
    $hasCorpus = (Test-Path $corpusDir) -and
        ((Get-ChildItem -Path $corpusDir -File -ErrorAction SilentlyContinue | Measure-Object).Count -gt 0)

    if ($hasCorpus) {
        # Replay = pass the individual corpus FILES (not the directory). With a
        # directory arg libFuzzer fuzzes indefinitely and writes new inputs into
        # it; passing files runs each exactly once and exits -- deterministic
        # regression replay.
        $corpusFiles = Get-ChildItem -Path $corpusDir -File | Select-Object -ExpandProperty FullName
        $args = @($corpusFiles) + @("-timeout=$TimeoutSec")
        $mode = "replay ($($corpusFiles.Count) inputs)"
    } else {
        # No committed corpus yet: smoke-run so a link/load/ASan-init regression
        # is still caught even before seeds exist.
        $args = @('-runs=0', "-timeout=$TimeoutSec")
        $mode = 'smoke'
    }

    Write-Host "==> $name ($mode): $($exe.Name)"
    & $exe.FullName @args
    $code = $LASTEXITCODE

    $status = if ($code -eq 0) { 'PASS' } else { 'FAIL'; }
    if ($code -ne 0) { $anyFailed = $true }
    $results.Add([pscustomobject]@{ Target = $name; Mode = $mode; Exit = $code; Status = $status })
}

Write-Host ''
Write-Host '=== Fuzz replay summary ==='
$results | Format-Table -AutoSize | Out-String | Write-Host

if ($anyFailed) {
    Write-Error 'One or more fuzz targets failed replay.'
    exit 1
}
Write-Host "All $($results.Count) fuzz target(s) passed replay."
exit 0
