<#
.SYNOPSIS
    Single-command repo-wide test orchestrator. The local "definition of green"
    for the Godot for XBOX on PC repo (this project intentionally has no CI).

.DESCRIPTION
    Pipeline (each stage gates the next; a failure aborts downstream work):

      1. Parse gate          -- tools\check_gd_scripts_headless.ps1
      2. CMake build         -- cmake --build build --preset debug   (skippable)
      3. C++ doctest         -- build\bin\Debug\gdk_unit_tests.exe
      4. GUT host runs       -- per coverage host:
                                  a. one-time `--headless --import` (marker file)
                                  b. `--headless -s res://addons/gut/gut_cmdln.gd
                                      -gdir=res://tests -gexit`
      5. PlayFab Multiplayer live orchestration -- opt-in multi-client smoke
      6. Bootstrap runners   -- `<host>\tests\bootstrap\*.gd` if present
      7. Aggregate           -- writes <OutDir>\run-summary.{json,md}

    Environment propagation goes through [System.Diagnostics.ProcessStartInfo]
    with UseShellExecute = $false. The orchestrator NEVER mutates $env:* in the
    parent shell. Async stdout/stderr drain is required (sync ReadToEnd() will
    deadlock once Godot floods stderr).

    GUT exits 0 even when zero tests are discovered. The orchestrator parses
    GUT's own summary block and asserts Tests > 0 per host; otherwise a
    misconfigured `-gdir` would silently be reported as green.

.PARAMETER Live
    Sets LIVE_TESTS=1 in the child env for every Godot stage. Live tests may
    talk to services and mutate online state.

.PARAMETER SkipBuild
    Skips the CMake build stage. The doctest exe and the GUT mirrored copies
    must already exist from a prior build.

.PARAMETER OutDir
    Directory for run-summary.{json,md}. Created if missing. Default:
    build\test-results.

.PARAMETER Hosts
    Optional filter of GUT host project roots (relative to repo root). Default
    is all three coverage hosts: tests\godot\gdk, tests\godot\playfab,
    tests\godot\gameinput.

.PARAMETER ParseProjects
    Optional project/context filter forwarded to the parse gate. Uses the same
    matching rules as tools\check_gd_scripts_headless.ps1 -Projects.

.PARAMETER ParseExcludeProjects
    Optional project/context exclusion forwarded to the parse gate. For example,
    pass `-ParseExcludeProjects tests\godot\playfab` to keep the parse gate
    active while skipping the PlayFab test host.

.PARAMETER PlayFabTitleId
    Optional PlayFab title id forwarded to Godot children as PLAYFAB_TITLE_ID.
    The PlayFab test base applies it to ProjectSettings['playfab/runtime/title_id'].

.PARAMETER PlayFabCustomId
    Optional pre-existing PlayFab custom id forwarded to Godot children as
    PLAYFAB_CUSTOM_ID. Live custom-ID tests sign in with create_account=false.

.PARAMETER PlayFabMatchmakingQueue
    Optional PlayFab matchmaking queue name forwarded to child processes as
    PLAYFAB_MULTIPLAYER_MATCH_QUEUE for Multiplayer live smoke coverage.

.PARAMETER GutTimeoutSec
    Per-host GUT and per-bootstrap-script timeout in seconds. Default: 600.

.PARAMETER VerboseOutput
    Streams child stdout/stderr to the host console as it arrives.

.OUTPUTS
    Writes <OutDir>\run-summary.json and <OutDir>\run-summary.md.
    Exits 0 on overall pass, 1 otherwise.
#>
[CmdletBinding()]
param(
    [switch]$Live,
    [switch]$SkipBuild,
    [string]$OutDir = 'build/test-results',
    [string[]]$Hosts,
    [string[]]$ParseProjects,
    [string[]]$ParseExcludeProjects,
    [string]$PlayFabTitleId,
    [string]$PlayFabCustomId,
    [string]$PlayFabMatchmakingQueue,
    [int]$GutTimeoutSec = 600,
    [switch]$VerboseOutput
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# ------------------------------------------------------------------------
# Constants
# ------------------------------------------------------------------------

$script:RepoRoot = [System.IO.Path]::GetFullPath((& git rev-parse --show-toplevel).Trim())
$script:DefaultHosts = @(
    'tests\godot\gdk',
    'tests\godot\playfab',
    'tests\godot\gameinput'
)
$script:DoctestExe = Join-Path $script:RepoRoot 'build\bin\Debug\gdk_unit_tests.exe'
$script:ParseGate  = Join-Path $script:RepoRoot 'tools\check_gd_scripts_headless.ps1'
$script:PlayFabMultiplayerLiveRunner = Join-Path $script:RepoRoot 'tools\run_playfab_multiplayer_live.ps1'

# GUT summary line regex. GUT (`addons/gut/summary.gd::_total_fmt`) renders each
# total as <label rpad 18><value lpad 5>. Values are integers, the literal
# "none" (when the count is zero), or for `Asserts` the form "<pass>/<total>"
# when at least one assert failed. Labels emitted only when non-zero are
# `Failing Tests`, `Risky/Pending`, `Orphans`. Wave 4 / docs may reference this
# regex to keep summary parsing in one place.
$script:GutSummaryRegex = '^\s*(?<label>Scripts|Tests|Passing Tests|Failing Tests|Risky/Pending|Asserts|Orphans|Time)\s+(?<value>\d+(?:/\d+)?|none|[\d\.]+s)\s*$'

# ------------------------------------------------------------------------
# Godot discovery (mirrors tools\check_gd_scripts_headless.ps1)
# ------------------------------------------------------------------------

function Get-GodotExecutable {
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

    throw "Could not find a Godot executable. Set GODOT_CONSOLE / GODOT_BIN / GODOT, or place a Godot console executable under sample\."
}

function Get-GodotVersion {
    param([Parameter(Mandatory = $true)][string]$GodotExe)
    try {
        $output = & $GodotExe --version 2>&1 | Select-Object -First 5
        foreach ($line in $output) {
            $m = [regex]::Match([string]$line, '(\d+\.\d+(?:\.\d+)?[A-Za-z0-9\.\-_]*)')
            if ($m.Success -and $m.Value -match '^4\.') { return $m.Value }
        }
        return ([string]($output | Select-Object -Last 1)).Trim()
    } catch {
        return 'unknown'
    }
}

# ------------------------------------------------------------------------
# Process invocation
#
# Critical contract (verified by Wave -1 spike, see spike-report.md sections
# 2 and 3):
#   - Env vars MUST be applied via $psi.EnvironmentVariables[k] = v with
#     UseShellExecute = $false. `$env:NAME = ...` in the parent shell does
#     NOT propagate.
#   - stdout/stderr MUST be drained asynchronously. Sync ReadToEnd()
#     deadlocks once Godot floods stderr (e.g. missing GDExtension noise).
# ------------------------------------------------------------------------

function Invoke-ChildProcess {
    param(
        [Parameter(Mandatory = $true)][string]$FileName,
        [AllowEmptyCollection()][string[]]$Arguments = @(),
        [Parameter(Mandatory = $true)][string]$WorkingDirectory,
        [hashtable]$EnvOverrides = @{},
        [int]$TimeoutSec = 600,
        [switch]$Stream
    )

    $psi = [System.Diagnostics.ProcessStartInfo]::new()
    $psi.FileName = $FileName
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    $psi.CreateNoWindow = $true

    foreach ($a in $Arguments) { [void]$psi.ArgumentList.Add($a) }

    # Inherit current environment, then layer overrides on top. ProcessStartInfo
    # starts with the parent env when UseShellExecute = $false, but we
    # re-apply explicitly for two reasons: (1) defence-in-depth on weird
    # PowerShell hosts, (2) to make the test surface deterministic.
    foreach ($entry in [System.Environment]::GetEnvironmentVariables().GetEnumerator()) {
        $psi.EnvironmentVariables[[string]$entry.Key] = [string]$entry.Value
    }
    $psi.EnvironmentVariables.Remove('PLAYFAB_DEVELOPER_SECRET_KEY')
    foreach ($k in $EnvOverrides.Keys) {
        $psi.EnvironmentVariables[[string]$k] = [string]$EnvOverrides[$k]
    }

    $proc = [System.Diagnostics.Process]::new()
    $proc.StartInfo = $psi

    $stdout = [System.Text.StringBuilder]::new()
    $stderr = [System.Text.StringBuilder]::new()

    # Closure captures by reference. Stream switch is captured into a local
    # so the event handler doesn't need to reach back into the param block.
    $streamLocal = $Stream.IsPresent
    $stdoutHandler = {
        if ($null -ne $EventArgs.Data) {
            [void]$Event.MessageData.StdoutBuilder.AppendLine($EventArgs.Data)
            if ($Event.MessageData.Stream) {
                Write-Host $EventArgs.Data
            }
        }
    }
    $stderrHandler = {
        if ($null -ne $EventArgs.Data) {
            [void]$Event.MessageData.StderrBuilder.AppendLine($EventArgs.Data)
            if ($Event.MessageData.Stream) {
                Write-Host $EventArgs.Data -ForegroundColor Yellow
            }
        }
    }

    $messageData = [pscustomobject]@{
        StdoutBuilder = $stdout
        StderrBuilder = $stderr
        Stream        = $streamLocal
    }

    $stdoutSub = Register-ObjectEvent -InputObject $proc -EventName 'OutputDataReceived' `
        -Action $stdoutHandler -MessageData $messageData
    $stderrSub = Register-ObjectEvent -InputObject $proc -EventName 'ErrorDataReceived' `
        -Action $stderrHandler -MessageData $messageData

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $timedOut = $false
    try {
        [void]$proc.Start()
        $proc.BeginOutputReadLine()
        $proc.BeginErrorReadLine()

        if (-not $proc.WaitForExit($TimeoutSec * 1000)) {
            $timedOut = $true
            try { $proc.Kill($true) } catch { }
            $proc.WaitForExit()
        } else {
            # Wait once more (no timeout) to ensure the async readers drain
            # everything that was buffered before exit. WaitForExit() with
            # a timeout does not guarantee the OutputDataReceived event has
            # delivered its terminal null sentinel.
            $proc.WaitForExit()
        }
    } finally {
        $sw.Stop()
        try { Unregister-Event -SourceIdentifier $stdoutSub.Name -ErrorAction SilentlyContinue } catch { }
        try { Unregister-Event -SourceIdentifier $stderrSub.Name -ErrorAction SilentlyContinue } catch { }
        try { Remove-Job -Job $stdoutSub -Force -ErrorAction SilentlyContinue } catch { }
        try { Remove-Job -Job $stderrSub -Force -ErrorAction SilentlyContinue } catch { }
    }

    return [pscustomobject]@{
        ExitCode    = if ($timedOut) { -1 } else { $proc.ExitCode }
        Stdout      = $stdout.ToString()
        Stderr      = $stderr.ToString()
        DurationMs  = [int]$sw.Elapsed.TotalMilliseconds
        TimedOut    = $timedOut
    }
}

# ------------------------------------------------------------------------
# GUT summary parsing
# ------------------------------------------------------------------------

function ConvertTo-GutInt {
    param([string]$Raw)
    if ($null -eq $Raw) { return 0 }
    if ($Raw -ieq 'none') { return 0 }
    if ($Raw -match '^(\d+)$') { return [int]$Matches[1] }
    if ($Raw -match '^(\d+)/(\d+)$') { return [int]$Matches[2] }  # asserts total
    return 0
}

function Parse-GutSummary {
    param([Parameter(Mandatory = $true)][string]$Text)

    $result = @{
        Tests        = $null
        Passing      = $null
        Failing      = 0
        Pending      = 0
        Asserts      = $null
        AssertsPass  = $null
        Orphans      = 0
        FoundSummary = $false
        NothingRun   = $false
    }

    if ($Text -match 'Nothing was run\.') {
        $result.NothingRun = $true
    }

    foreach ($line in ($Text -split "`r?`n")) {
        if ($line -match $script:GutSummaryRegex) {
            $result.FoundSummary = $true
            $label = $Matches['label']
            $value = $Matches['value']
            switch ($label) {
                'Tests'         { $result.Tests   = ConvertTo-GutInt $value }
                'Passing Tests' { $result.Passing = ConvertTo-GutInt $value }
                'Failing Tests' { $result.Failing = ConvertTo-GutInt $value }
                'Risky/Pending' { $result.Pending = ConvertTo-GutInt $value }
                'Asserts'       {
                    if ($value -match '^(\d+)/(\d+)$') {
                        $result.AssertsPass = [int]$Matches[1]
                        $result.Asserts     = [int]$Matches[2]
                    } elseif ($value -ieq 'none') {
                        $result.AssertsPass = 0
                        $result.Asserts     = 0
                    } else {
                        $result.AssertsPass = [int]$value
                        $result.Asserts     = [int]$value
                    }
                }
                'Orphans'       { $result.Orphans = ConvertTo-GutInt $value }
                default { }
            }
        }
    }

    return $result
}

# ------------------------------------------------------------------------
# Stage helpers
# ------------------------------------------------------------------------

function New-StageRecord {
    param([string]$Name)
    return [ordered]@{
        name        = $Name
        status      = 'skip'
        duration_ms = 0
        exit_code   = $null
        tests       = $null
        passing     = $null
        failing     = $null
        pending     = $null
        asserts     = $null
        asserts_pass = $null
        message     = $null
        details     = $null
    }
}

function Resolve-PwshExecutable {
    $candidates = @()
    if ($PSVersionTable.PSEdition -eq 'Core' -and -not [string]::IsNullOrWhiteSpace([System.Environment]::ProcessPath)) {
        $candidates += [System.Environment]::ProcessPath
    }
    foreach ($name in @('pwsh', 'pwsh.exe', 'powershell', 'powershell.exe')) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($null -ne $cmd -and -not [string]::IsNullOrWhiteSpace($cmd.Source)) {
            $candidates += $cmd.Source
        }
    }
    foreach ($c in ($candidates | Select-Object -Unique)) {
        if (Test-Path $c) { return $c }
    }
    throw 'Could not locate a PowerShell executable for the parse-gate stage.'
}

function Resolve-CMakeExecutable {
    $cmd = Get-Command 'cmake' -ErrorAction SilentlyContinue
    if ($null -eq $cmd -or [string]::IsNullOrWhiteSpace($cmd.Source)) {
        throw 'cmake not found on PATH; cannot run the build stage.'
    }
    return $cmd.Source
}

function ConvertTo-ParseGateFilterList {
    param(
        [AllowEmptyCollection()]
        [string[]]$Filters
    )

    return @(
        $Filters |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            ForEach-Object { $_ -split ',' } |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            ForEach-Object { ($_ -replace '/', '\').Trim().TrimEnd('\') } |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            Sort-Object -Unique
    )
}

function Invoke-ParseGate {
    param(
        [AllowEmptyCollection()]
        [string[]]$Projects = @(),
        [AllowEmptyCollection()]
        [string[]]$ExcludeProjects = @()
    )

    $rec = New-StageRecord 'parse-gate'
    $pwsh = Resolve-PwshExecutable
    $args = @('-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $script:ParseGate)
    if ($Projects.Count -gt 0) {
        $args += @('-Projects', ($Projects -join ','))
    }
    if ($ExcludeProjects.Count -gt 0) {
        $args += @('-ExcludeProjects', ($ExcludeProjects -join ','))
    }
    $r = Invoke-ChildProcess -FileName $pwsh -Arguments $args -WorkingDirectory $script:RepoRoot `
        -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
    $rec.duration_ms = $r.DurationMs
    $rec.exit_code   = $r.ExitCode
    $rec.status      = if ($r.ExitCode -eq 0) { 'pass' } else { 'fail' }
    $rec.message     = if ($r.TimedOut) { "Parse gate timed out after $GutTimeoutSec s" }
                       elseif ($r.ExitCode -ne 0) { "check_gd_scripts_headless.ps1 exited $($r.ExitCode)" }
                       else { 'OK' }
    $rec.details     = if ($r.ExitCode -eq 0) { $null } else { ($r.Stdout + $r.Stderr).Trim() }
    return $rec
}

function Invoke-Build {
    $rec = New-StageRecord 'cmake-build'
    if ($SkipBuild) {
        $rec.status  = 'skip'
        $rec.message = 'Skipped (-SkipBuild).'
        return $rec
    }
    $cmake = Resolve-CMakeExecutable

    # Configure if the build dir is missing. `cmake --preset default` is the
    # repo-wide configure preset documented in copilot-instructions.md. We
    # tolerate a pre-configured build/ from earlier sessions.
    if (-not (Test-Path (Join-Path $script:RepoRoot 'build\CMakeCache.txt'))) {
        $cfg = Invoke-ChildProcess -FileName $cmake -Arguments @('--preset', 'default') `
            -WorkingDirectory $script:RepoRoot -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
        if ($cfg.ExitCode -ne 0) {
            $rec.duration_ms = $cfg.DurationMs
            $rec.exit_code   = $cfg.ExitCode
            $rec.status      = 'fail'
            $rec.message     = "cmake --preset default failed (exit $($cfg.ExitCode))."
            $rec.details     = ($cfg.Stdout + $cfg.Stderr).Trim()
            return $rec
        }
    }

    $bld = Invoke-ChildProcess -FileName $cmake -Arguments @('--build', 'build', '--preset', 'debug') `
        -WorkingDirectory $script:RepoRoot -TimeoutSec ($GutTimeoutSec * 4) -Stream:$VerboseOutput
    $rec.duration_ms = $bld.DurationMs
    $rec.exit_code   = $bld.ExitCode
    $rec.status      = if ($bld.ExitCode -eq 0) { 'pass' } else { 'fail' }
    $rec.message     = if ($bld.TimedOut) { 'Build timed out.' }
                       elseif ($bld.ExitCode -ne 0) { "cmake --build failed (exit $($bld.ExitCode))." }
                       else { 'OK' }
    $rec.details     = if ($bld.ExitCode -eq 0) { $null } else { ($bld.Stdout + $bld.Stderr).Trim() }
    return $rec
}

function Invoke-Doctest {
    $rec = New-StageRecord 'cpp-doctest'
    if (-not (Test-Path $script:DoctestExe)) {
        $rec.status  = 'fail'
        $rec.message = "Doctest exe not found at $script:DoctestExe; run cmake build first or drop -SkipBuild."
        return $rec
    }
    $r = Invoke-ChildProcess -FileName $script:DoctestExe -Arguments @() `
        -WorkingDirectory $script:RepoRoot -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
    $rec.duration_ms = $r.DurationMs
    $rec.exit_code   = $r.ExitCode
    $rec.status      = if ($r.ExitCode -eq 0) { 'pass' } else { 'fail' }
    $rec.message     = if ($r.TimedOut) { 'Doctest run timed out.' }
                       elseif ($r.ExitCode -ne 0) { "gdk_unit_tests exited $($r.ExitCode)." }
                       else { 'OK' }
    $rec.details     = ($r.Stdout + ($(if ($r.Stderr) { "`n--- stderr ---`n" + $r.Stderr } else { '' }))).Trim()
    return $rec
}

function Ensure-HostImported {
    param(
        [Parameter(Mandatory = $true)][string]$HostRoot,
        [Parameter(Mandatory = $true)][string]$GodotExe,
        [Parameter(Mandatory = $true)][hashtable]$ChildEnv
    )
    $marker = Join-Path $HostRoot '.godot\orchestrator-imported'
    if (Test-Path $marker) { return $null }
    $r = Invoke-ChildProcess -FileName $GodotExe -Arguments @('--headless', '--import') `
        -WorkingDirectory $HostRoot -EnvOverrides $ChildEnv -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
    if ($r.ExitCode -ne 0) {
        return [pscustomobject]@{
            ExitCode = $r.ExitCode
            Output   = ($r.Stdout + $r.Stderr).Trim()
            TimedOut = $r.TimedOut
        }
    }
    $markerDir = Split-Path -Parent $marker
    if (-not (Test-Path $markerDir)) {
        New-Item -ItemType Directory -Force -Path $markerDir | Out-Null
    }
    Set-Content -Path $marker -Value ("imported at " + (Get-Date -Format 'o')) -Encoding ASCII
    return $null
}

function Invoke-GutHost {
    param(
        [Parameter(Mandatory = $true)][string]$RelativeHost,
        [Parameter(Mandatory = $true)][string]$GodotExe,
        [Parameter(Mandatory = $true)][hashtable]$ChildEnv
    )
    $rec = New-StageRecord ("gut:" + ($RelativeHost -replace '\\','/'))
    $hostRoot = Join-Path $script:RepoRoot $RelativeHost
    if (-not (Test-Path (Join-Path $hostRoot 'project.godot'))) {
        $rec.status  = 'fail'
        $rec.message = "Host '$RelativeHost' has no project.godot."
        return $rec
    }
    if (-not (Test-Path (Join-Path $hostRoot 'addons\gut\gut_cmdln.gd'))) {
        $rec.status  = 'fail'
        $rec.message = "GUT not mirrored into '$RelativeHost\addons\gut\'. Did the build stage run? (cmake --build refreshes mirrored copies.)"
        return $rec
    }
    if (-not (Test-Path (Join-Path $hostRoot 'tests'))) {
        $rec.status  = 'fail'
        $rec.message = "Host '$RelativeHost' has no tests\ directory."
        return $rec
    }

    $importErr = Ensure-HostImported -HostRoot $hostRoot -GodotExe $GodotExe -ChildEnv $ChildEnv
    if ($null -ne $importErr) {
        $rec.status    = 'fail'
        $rec.exit_code = $importErr.ExitCode
        $rec.message   = "One-time '--headless --import' for $RelativeHost failed (exit $($importErr.ExitCode))."
        $rec.details   = $importErr.Output
        return $rec
    }

    $args = @('--headless', '-s', 'res://addons/gut/gut_cmdln.gd', '-gdir=res://tests', '-ginclude_subdirs', '-gexit')
    $r = Invoke-ChildProcess -FileName $GodotExe -Arguments $args -WorkingDirectory $hostRoot `
        -EnvOverrides $ChildEnv -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput

    $rec.duration_ms = $r.DurationMs
    $rec.exit_code   = $r.ExitCode

    $combined = $r.Stdout + "`n" + $r.Stderr
    $summary = Parse-GutSummary -Text $combined
    $rec.tests   = $summary.Tests
    $rec.passing = $summary.Passing
    $rec.failing = $summary.Failing
    $rec.pending = $summary.Pending
    $rec.asserts = $summary.Asserts
    $rec.asserts_pass = $summary.AssertsPass

    if ($r.TimedOut) {
        $rec.status  = 'fail'
        $rec.message = "GUT timed out in $RelativeHost after $GutTimeoutSec s."
        $rec.details = $combined.Trim()
        return $rec
    }
    if ($summary.NothingRun -or -not $summary.FoundSummary) {
        $rec.status  = 'fail'
        $rec.message = "GUT discovered no tests in '$RelativeHost' -- check -gdir or that test files match the GUT discovery pattern (default: test_*.gd)."
        $rec.details = $combined.Trim()
        return $rec
    }
    if ($null -eq $summary.Tests -or $summary.Tests -le 0) {
        $rec.status  = 'fail'
        $rec.message = "GUT discovered no tests in '$RelativeHost' -- check -gdir or that test files match the GUT discovery pattern (default: test_*.gd)."
        $rec.details = $combined.Trim()
        return $rec
    }
    if ($summary.Failing -gt 0 -or $r.ExitCode -ne 0) {
        $rec.status  = 'fail'
        $rec.message = "GUT failed in '$RelativeHost' -- $($summary.Failing) failing test(s), exit $($r.ExitCode)."
        $rec.details = $combined.Trim()
        return $rec
    }

    $rec.status  = 'pass'
    $assertsTotal  = if ($null -eq $summary.Asserts)     { 0 } else { $summary.Asserts }
    $assertsPass   = if ($null -eq $summary.AssertsPass) { 0 } else { $summary.AssertsPass }
    $assertsFailed = $assertsTotal - $assertsPass
    $rec.message = "OK ($($summary.Tests) test(s), $($summary.Passing) passing, $($summary.Pending) pending, $assertsPass/$assertsTotal asserts validated, $assertsFailed failed)."
    return $rec
}

function Invoke-BootstrapRunners {
    param(
        [Parameter(Mandatory = $true)][string]$RelativeHost,
        [Parameter(Mandatory = $true)][string]$GodotExe,
        [Parameter(Mandatory = $true)][hashtable]$ChildEnv
    )
    $records = @()
    $hostRoot = Join-Path $script:RepoRoot $RelativeHost
    $bootstrapDir = Join-Path $hostRoot 'tests\bootstrap'
    if (-not (Test-Path $bootstrapDir)) {
        $rec = New-StageRecord ("bootstrap:" + ($RelativeHost -replace '\\','/'))
        $rec.status  = 'skip'
        $rec.message = "no bootstrap suites (tests\bootstrap\ does not exist)"
        return ,@($rec)
    }
    $scripts = Get-ChildItem -Path $bootstrapDir -Filter '*.gd' -File -ErrorAction SilentlyContinue |
        Sort-Object Name
    if ($scripts.Count -eq 0) {
        $rec = New-StageRecord ("bootstrap:" + ($RelativeHost -replace '\\','/'))
        $rec.status  = 'skip'
        $rec.message = "no bootstrap suites (tests\bootstrap\ is empty)"
        return ,@($rec)
    }

    foreach ($s in $scripts) {
        $rec = New-StageRecord ("bootstrap:" + ($RelativeHost -replace '\\','/') + ":" + $s.BaseName)
        $resPath = 'res://tests/bootstrap/' + $s.Name
        $args = @('--headless', '--script', $resPath)
        $r = Invoke-ChildProcess -FileName $GodotExe -Arguments $args -WorkingDirectory $hostRoot `
            -EnvOverrides $ChildEnv -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
        $rec.duration_ms = $r.DurationMs
        $rec.exit_code   = $r.ExitCode
        if ($r.TimedOut) {
            $rec.status  = 'fail'
            $rec.message = "Bootstrap '$($s.Name)' in $RelativeHost timed out."
        } elseif ($r.ExitCode -ne 0) {
            $rec.status  = 'fail'
            $rec.message = "Bootstrap '$($s.Name)' in $RelativeHost exited $($r.ExitCode)."
        } else {
            $rec.status  = 'pass'
            $rec.message = 'OK'
        }
        if ($rec.status -eq 'fail') {
            $rec.details = ($r.Stdout + "`n--- stderr ---`n" + $r.Stderr).Trim()
        }
        $records += $rec
    }
    return ,$records
}

function Invoke-PlayFabMultiplayerLive {
    param(
        [Parameter(Mandatory = $true)][string]$GodotExe,
        [Parameter(Mandatory = $true)][hashtable]$ChildEnv,
        [Parameter(Mandatory = $true)][string[]]$HostList,
        [Parameter(Mandatory = $true)][bool]$LiveEnabled,
        [Parameter(Mandatory = $true)][string]$OutDirAbsolute
    )

    $rec = New-StageRecord 'playfab-multiplayer-live'
    if (-not $LiveEnabled) {
        $rec.status = 'skip'
        $rec.message = 'Skipped without -Live / LIVE_TESTS=1.'
        return $rec
    }
    if (-not ($HostList -contains 'tests\godot\playfab')) {
        $rec.status = 'skip'
        $rec.message = 'Skipped (-Hosts filter excluded tests\godot\playfab).'
        return $rec
    }
    if (-not (Test-Path $script:PlayFabMultiplayerLiveRunner)) {
        $rec.status = 'fail'
        $rec.message = "PlayFab Multiplayer live runner not found at $script:PlayFabMultiplayerLiveRunner."
        return $rec
    }

    $pwsh = Resolve-PwshExecutable
    $args = @(
        '-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass',
        '-File', $script:PlayFabMultiplayerLiveRunner,
        '-RepoRoot', $script:RepoRoot,
        '-GodotExe', $GodotExe,
        '-OutDir', $OutDirAbsolute,
        '-TimeoutSec', ([string]$GutTimeoutSec)
    )
    if ($VerboseOutput) {
        $args += '-VerboseOutput'
    }

    $r = Invoke-ChildProcess -FileName $pwsh -Arguments $args -WorkingDirectory $script:RepoRoot `
        -EnvOverrides $ChildEnv -TimeoutSec ($GutTimeoutSec * 2) -Stream:$VerboseOutput
    $rec.duration_ms = $r.DurationMs
    $rec.exit_code = $r.ExitCode
    $combined = ($r.Stdout + "`n" + $r.Stderr).Trim()
    $rec.details = if ($r.ExitCode -eq 0) { $null } else { $combined }

    if ($r.TimedOut) {
        $rec.status = 'fail'
        $rec.message = "PlayFab Multiplayer live orchestration timed out after $($GutTimeoutSec * 2) s."
    } elseif ($r.ExitCode -ne 0) {
        $rec.status = 'fail'
        $rec.message = "PlayFab Multiplayer live orchestration failed (exit $($r.ExitCode))."
    } elseif ($combined -match '(?m)^SKIP:\s*(?<message>.+)$') {
        $rec.status = 'skip'
        $rec.message = $Matches['message']
    } else {
        $rec.status = 'pass'
        $rec.message = if ([string]::IsNullOrWhiteSpace($combined)) { 'OK' } else { ($combined -split "`r?`n" | Select-Object -Last 1) }
    }
    return $rec
}

# ------------------------------------------------------------------------
# Aggregation / output
# ------------------------------------------------------------------------

function Write-RunSummary {
    param(
        [Parameter(Mandatory = $true)][System.Collections.IList]$Stages,
        [Parameter(Mandatory = $true)][string]$OutDirAbsolute,
        [Parameter(Mandatory = $true)][string]$OverallStatus,
        [Parameter(Mandatory = $true)][datetime]$StartedAtUtc,
        [Parameter(Mandatory = $true)][datetime]$FinishedAtUtc,
        [Parameter(Mandatory = $true)][bool]$LiveFlag,
        [Parameter(Mandatory = $true)][string]$GodotVersion
    )

    if (-not (Test-Path $OutDirAbsolute)) {
        New-Item -ItemType Directory -Force -Path $OutDirAbsolute | Out-Null
    }

    $totalMs = [int](($FinishedAtUtc - $StartedAtUtc).TotalMilliseconds)

    # JSON
    $payload = [ordered]@{
        overall_status    = $OverallStatus
        started_at        = $StartedAtUtc.ToString("o")
        finished_at       = $FinishedAtUtc.ToString("o")
        total_duration_ms = $totalMs
        live              = $LiveFlag
        godot_version     = $GodotVersion
        stages            = @($Stages)
    }
    $jsonPath = Join-Path $OutDirAbsolute 'run-summary.json'
    $payload | ConvertTo-Json -Depth 8 | Set-Content -Path $jsonPath -Encoding UTF8

    # Markdown
    $mdLines = New-Object System.Collections.Generic.List[string]
    $statusEmoji = if ($OverallStatus -eq 'pass') { 'OK' } else { 'FAIL' }
    [void]$mdLines.Add("# run_all_tests summary -- $statusEmoji")
    [void]$mdLines.Add('')
    [void]$mdLines.Add("- **Overall**: ``$OverallStatus``")
    [void]$mdLines.Add("- **Started (UTC)**: $($StartedAtUtc.ToString('o'))")
    [void]$mdLines.Add("- **Finished (UTC)**: $($FinishedAtUtc.ToString('o'))")
    [void]$mdLines.Add("- **Duration**: ${totalMs} ms")
    [void]$mdLines.Add("- **Live**: $LiveFlag")
    [void]$mdLines.Add("- **Godot**: $GodotVersion")
    [void]$mdLines.Add('')
    [void]$mdLines.Add('| Stage | Status | Duration (ms) | Exit | Tests | Pass | Fail | Pend | Asserts Validated | Asserts Failed |')
    [void]$mdLines.Add('|-------|--------|---------------|------|-------|------|------|------|-------------------|----------------|')
    foreach ($s in $Stages) {
        $assertsValidated = '-'
        $assertsFailed    = '-'
        if ($null -ne $s.asserts) {
            $totalA  = [int]$s.asserts
            $passA   = if ($null -eq $s.asserts_pass) { $totalA } else { [int]$s.asserts_pass }
            $assertsValidated = "$passA/$totalA"
            $assertsFailed    = $totalA - $passA
        }
        $row = '| {0} | {1} | {2} | {3} | {4} | {5} | {6} | {7} | {8} | {9} |' -f `
            $s.name,
            $s.status,
            $s.duration_ms,
            ($(if ($null -eq $s.exit_code) { '-' } else { $s.exit_code })),
            ($(if ($null -eq $s.tests)     { '-' } else { $s.tests })),
            ($(if ($null -eq $s.passing)   { '-' } else { $s.passing })),
            ($(if ($null -eq $s.failing)   { '-' } else { $s.failing })),
            ($(if ($null -eq $s.pending)   { '-' } else { $s.pending })),
            $assertsValidated,
            $assertsFailed
        [void]$mdLines.Add($row)
    }
    [void]$mdLines.Add('')

    foreach ($s in $Stages) {
        [void]$mdLines.Add("## $($s.name)")
        [void]$mdLines.Add('')
        [void]$mdLines.Add("- status: ``$($s.status)``")
        if ($null -ne $s.exit_code)  { [void]$mdLines.Add("- exit_code: ``$($s.exit_code)``") }
        [void]$mdLines.Add("- duration_ms: $($s.duration_ms)")
        if ($null -ne $s.tests)   { [void]$mdLines.Add("- tests: $($s.tests)") }
        if ($null -ne $s.passing) { [void]$mdLines.Add("- passing: $($s.passing)") }
        if ($null -ne $s.failing) { [void]$mdLines.Add("- failing: $($s.failing)") }
        if ($null -ne $s.pending) { [void]$mdLines.Add("- pending: $($s.pending)") }
        if ($null -ne $s.asserts) {
            $totalA = [int]$s.asserts
            $passA  = if ($null -eq $s.asserts_pass) { $totalA } else { [int]$s.asserts_pass }
            [void]$mdLines.Add("- asserts validated: $passA/$totalA")
            [void]$mdLines.Add("- asserts failed: $($totalA - $passA)")
        }
        if ($s.message) { [void]$mdLines.Add("- message: $($s.message)") }
        if ($s.details) {
            $excerpt = $s.details
            if ($excerpt.Length -gt 4000) { $excerpt = $excerpt.Substring($excerpt.Length - 4000) }
            [void]$mdLines.Add('')
            [void]$mdLines.Add('```text')
            foreach ($line in ($excerpt -split "`r?`n")) { [void]$mdLines.Add($line) }
            [void]$mdLines.Add('```')
        }
        [void]$mdLines.Add('')
    }

    $mdPath = Join-Path $OutDirAbsolute 'run-summary.md'
    Set-Content -Path $mdPath -Value ($mdLines -join "`n") -Encoding UTF8

    return [pscustomobject]@{
        JsonPath = $jsonPath
        MdPath   = $mdPath
    }
}

# ------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------

function Main {
    $startedAt = (Get-Date).ToUniversalTime()
    $godotExe  = Get-GodotExecutable
    $godotVer  = Get-GodotVersion -GodotExe $godotExe

    $childEnv = @{}
    if ($Live) { $childEnv['LIVE_TESTS'] = '1' }
    if (-not [string]::IsNullOrWhiteSpace($PlayFabTitleId)) {
        $childEnv['PLAYFAB_TITLE_ID'] = $PlayFabTitleId.Trim()
    }
    if (-not [string]::IsNullOrWhiteSpace($PlayFabCustomId)) {
        $childEnv['PLAYFAB_CUSTOM_ID'] = $PlayFabCustomId.Trim()
    }
    if (-not [string]::IsNullOrWhiteSpace($PlayFabMatchmakingQueue)) {
        $childEnv['PLAYFAB_MULTIPLAYER_MATCH_QUEUE'] = $PlayFabMatchmakingQueue.Trim()
    }

    $hostList = if ($null -ne $Hosts -and $Hosts.Count -gt 0) { $Hosts } else { $script:DefaultHosts }
    # Normalize separators
    $hostList = @($hostList | ForEach-Object { ($_ -replace '/', '\').TrimEnd('\') })
    $parseProjectList = @(ConvertTo-ParseGateFilterList -Filters $ParseProjects)
    $parseExcludeProjectList = @(ConvertTo-ParseGateFilterList -Filters $ParseExcludeProjects)

    $outDirAbsolute = if ([System.IO.Path]::IsPathRooted($OutDir)) {
        $OutDir
    } else {
        Join-Path $script:RepoRoot $OutDir
    }

    Write-Host "run_all_tests.ps1: Godot = $godotExe ($godotVer)" -ForegroundColor Cyan
    Write-Host "                   Live  = $Live   SkipBuild = $SkipBuild" -ForegroundColor Cyan
    Write-Host "                   PlayFabTitleId = $(if ($childEnv.ContainsKey('PLAYFAB_TITLE_ID')) { 'set' } else { 'unset' })   PlayFabCustomId = $(if ($childEnv.ContainsKey('PLAYFAB_CUSTOM_ID')) { 'set' } else { 'unset' })   PlayFabMatchmakingQueue = $(if ($childEnv.ContainsKey('PLAYFAB_MULTIPLAYER_MATCH_QUEUE')) { 'set' } else { 'unset' })" -ForegroundColor Cyan
    Write-Host "                   Hosts = $($hostList -join ', ')" -ForegroundColor Cyan
    Write-Host "                   ParseProjects = $(if ($parseProjectList.Count -gt 0) { $parseProjectList -join ', ' } else { 'all' })" -ForegroundColor Cyan
    Write-Host "                   ParseExcludeProjects = $(if ($parseExcludeProjectList.Count -gt 0) { $parseExcludeProjectList -join ', ' } else { 'none' })" -ForegroundColor Cyan
    Write-Host "                   OutDir= $outDirAbsolute" -ForegroundColor Cyan
    Write-Host ''

    $stages = New-Object System.Collections.Generic.List[object]
    $abort = $false

    # 1. Parse gate
    Write-Host '== [1/7] Parse gate (check_gd_scripts_headless.ps1) ==' -ForegroundColor Cyan
    $stage = Invoke-ParseGate -Projects $parseProjectList -ExcludeProjects $parseExcludeProjectList
    [void]$stages.Add($stage)
    Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
    if ($stage.status -ne 'pass') { $abort = $true }

    # 2. Build
    if (-not $abort) {
        Write-Host '== [2/7] CMake build (debug) ==' -ForegroundColor Cyan
        $stage = Invoke-Build
        [void]$stages.Add($stage)
        Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
        if ($stage.status -eq 'fail') { $abort = $true }
    } else {
        $skip = New-StageRecord 'cmake-build'; $skip.message = 'Skipped (upstream stage failed).'; [void]$stages.Add($skip)
    }

    # 3. C++ doctest
    if (-not $abort) {
        Write-Host '== [3/7] C++ doctest (gdk_unit_tests.exe) ==' -ForegroundColor Cyan
        $stage = Invoke-Doctest
        [void]$stages.Add($stage)
        Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
        if ($stage.status -ne 'pass') { $abort = $true }
    } else {
        $skip = New-StageRecord 'cpp-doctest'; $skip.message = 'Skipped (upstream stage failed).'; [void]$stages.Add($skip)
    }

    # 4. GUT runs
    if (-not $abort) {
        Write-Host '== [4/7] GUT host runs ==' -ForegroundColor Cyan
        foreach ($h in $hostList) {
            Write-Host "  - host: $h"
            $stage = Invoke-GutHost -RelativeHost $h -GodotExe $godotExe -ChildEnv $childEnv
            [void]$stages.Add($stage)
            Write-Host "    $($stage.status.ToUpper()): $($stage.message)"
            if ($stage.status -eq 'fail') { $abort = $true }
        }
        Write-Host ''
    } else {
        foreach ($h in $hostList) {
            $skip = New-StageRecord ("gut:" + ($h -replace '\\','/'))
            $skip.message = 'Skipped (upstream stage failed).'
            [void]$stages.Add($skip)
        }
    }

    # 5. PlayFab Multiplayer live orchestration
    if (-not $abort) {
        Write-Host '== [5/7] PlayFab Multiplayer live orchestration ==' -ForegroundColor Cyan
        $stage = Invoke-PlayFabMultiplayerLive -GodotExe $godotExe -ChildEnv $childEnv -HostList $hostList -LiveEnabled:([bool]$Live) -OutDirAbsolute $outDirAbsolute
        [void]$stages.Add($stage)
        Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
        if ($stage.status -eq 'fail') { $abort = $true }
    } else {
        $skip = New-StageRecord 'playfab-multiplayer-live'
        $skip.message = 'Skipped (upstream stage failed).'
        [void]$stages.Add($skip)
    }

    # 6. Bootstrap mini-runners (run even if GUT failed? spec says abort on failure;
    # we honor the abort to keep the pipeline simple.)
    if (-not $abort) {
        Write-Host '== [6/7] Bootstrap mini-runners ==' -ForegroundColor Cyan
        $bootstrapHosts = @(@('tests\godot\gdk', 'tests\godot\gameinput', 'tests\godot\playfab') |
            Where-Object { $hostList -contains $_ })
        if ($bootstrapHosts.Count -eq 0) {
            $skip = New-StageRecord 'bootstrap'
            $skip.message = 'Skipped (-Hosts filter excluded all bootstrap-capable hosts).'
            [void]$stages.Add($skip)
            Write-Host "   SKIP: $($skip.message)"
        } else {
            foreach ($h in $bootstrapHosts) {
                Write-Host "  - host: $h"
                $records = Invoke-BootstrapRunners -RelativeHost $h -GodotExe $godotExe -ChildEnv $childEnv
                foreach ($rec in $records) {
                    [void]$stages.Add($rec)
                    Write-Host "    $($rec.status.ToUpper()): $($rec.name) -- $($rec.message)"
                    if ($rec.status -eq 'fail') { $abort = $true }
                }
            }
        }
        Write-Host ''
    } else {
        $skip = New-StageRecord 'bootstrap'
        $skip.message = 'Skipped (upstream stage failed).'
        [void]$stages.Add($skip)
    }

    # 7. Aggregate
    Write-Host '== [7/7] Aggregate run summary ==' -ForegroundColor Cyan
    $finishedAt = (Get-Date).ToUniversalTime()
    $overall = if ($stages | Where-Object { $_.status -eq 'fail' }) { 'fail' } else { 'pass' }
    $written = Write-RunSummary -Stages $stages -OutDirAbsolute $outDirAbsolute `
        -OverallStatus $overall -StartedAtUtc $startedAt -FinishedAtUtc $finishedAt `
        -LiveFlag:([bool]$Live) -GodotVersion $godotVer
    Write-Host "   wrote $($written.JsonPath)"
    Write-Host "   wrote $($written.MdPath)"
    Write-Host ''
    Write-Host "Overall: $overall" -ForegroundColor $(if ($overall -eq 'pass') { 'Green' } else { 'Red' })
    Write-Host "Summary: $($written.MdPath)"

    if ($overall -eq 'pass') { exit 0 } else { exit 1 }
}

Main
