#requires -Version 7.0
<#
.SYNOPSIS
  Run the MP test orchestrator with one or more spawned test clients.

.DESCRIPTION
  Wrapper that locates a Godot executable, then launches the headless
  mp_orchestrator Godot project. The orchestrator itself spawns the
  requested test client subprocesses, manages handshakes, and runs
  discovered scenarios.

  This wrapper handles only Godot discovery, arg plumbing, and exit-code
  propagation. The orchestrator owns scenario discovery, results writing,
  and child process lifecycle.

.PARAMETER Roles
  Comma-separated client roles to spawn. Default: "host".

.PARAMETER Port
  TCP port the orchestrator binds. Default: 18765.

.PARAMETER Filter
  Regex applied to SCENARIO_ID. Only matching scenarios run. Default: ".*".

.PARAMETER List
  Discover and list scenarios; do not run.

.PARAMETER ResultsDir
  Absolute directory for mp-test-results.{json,md}. Default: build\test-results\mp-test\<run-id>.

.PARAMETER ScenariosDir
  res:// path inside the orchestrator project that holds scenarios. Default: res://scenarios.

.PARAMETER NoSpawn
  Skip client subprocess spawning (wait for external clients to connect).

.PARAMETER ExtraClientArgs
  Extra args to append after `--` when launching each client.

.NOTES
  Discovery order for Godot matches tools/run_all_tests.ps1:
  GODOT_CONSOLE / GODOT_BIN / GODOT env, then sample\Godot*_console.exe,
  then `godot` / `godot4` on PATH.
#>
[CmdletBinding()]
param(
    [string]$Roles = 'host',
    [int]$Port = 18765,
    [string]$Filter = '.*',
    [switch]$List,
    [string]$ResultsDir = '',
    [string]$ScenariosDir = 'res://scenarios',
    [switch]$NoSpawn,
    [string[]]$ExtraClientArgs = @()
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$OrchestratorProject = Join-Path $RepoRoot 'tests\godot\mp_orchestrator'
$ClientProject = Join-Path $RepoRoot 'tests\godot\mp_test_client'

function Get-GodotExecutable {
    $candidates = [System.Collections.Generic.List[string]]::new()
    foreach ($envName in @('GODOT_CONSOLE', 'GODOT_BIN', 'GODOT')) {
        $value = [Environment]::GetEnvironmentVariable($envName)
        if (-not [string]::IsNullOrWhiteSpace($value)) { $candidates.Add($value) | Out-Null }
    }
    $sampleDir = Join-Path $RepoRoot 'sample'
    if (Test-Path $sampleDir) {
        foreach ($pattern in @('Godot*_console.exe', 'Godot*.exe')) {
            Get-ChildItem -Path $sampleDir -Filter $pattern -File -ErrorAction SilentlyContinue |
                Sort-Object FullName -Descending |
                ForEach-Object { $candidates.Add($_.FullName) | Out-Null }
        }
    }
    foreach ($commandName in @('godot', 'godot4')) {
        $cmd = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($null -ne $cmd -and -not [string]::IsNullOrWhiteSpace($cmd.Source)) {
            $candidates.Add($cmd.Source) | Out-Null
        }
    }
    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        if (Test-Path $candidate) {
            return [System.IO.Path]::GetFullPath((Resolve-Path $candidate).Path)
        }
    }
    throw "Could not find a Godot executable. Set GODOT_CONSOLE / GODOT_BIN / GODOT, or place a Godot console executable under sample\."
}

$Godot = Get-GodotExecutable
Write-Host "Using Godot: $Godot"
Write-Host "Orchestrator project: $OrchestratorProject"
Write-Host "Client project:       $ClientProject"

$orchArgs = @(
    '--headless',
    '--path', $OrchestratorProject,
    '--script', 'res://main.gd',
    '--',
    '--port', "$Port",
    '--role', $Roles,
    '--filter', $Filter,
    '--scenarios-dir', $ScenariosDir,
    '--client-godot', $Godot,
    '--client-project', $ClientProject
)
if ($List) { $orchArgs += '--list' }
if ($NoSpawn) { $orchArgs += '--no-spawn' }
if ($ResultsDir) { $orchArgs += @('--results-dir', $ResultsDir) }
foreach ($clientArg in $ExtraClientArgs) {
    $orchArgs += @('--client-arg', $clientArg)
}

Write-Host "Launching orchestrator..."
$env:MP_TEST_REPO_ROOT = $RepoRoot
$proc = Start-Process -FilePath $Godot -ArgumentList $orchArgs -NoNewWindow -PassThru -Wait
$exit = $proc.ExitCode
Write-Host "Orchestrator exit code: $exit"
exit $exit
