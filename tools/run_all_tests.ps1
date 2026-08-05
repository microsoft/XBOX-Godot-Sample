<#
.SYNOPSIS
    Single-command repo-wide test orchestrator. The local "definition of green"
    for the XBOX Godot Sample repo (this project intentionally has no CI).

.DESCRIPTION
    Pipeline (each stage gates the next; a failure aborts downstream work):

      0. Preflight           -- required live configuration is present, else abort
      1. Parse gate          -- tools\check_gd_scripts_headless.ps1
      2. CMake build         -- cmake --build build --preset debug   (skippable)
      3. C++ doctest         -- build\bin\Debug\gdk_unit_tests.exe
      4. Live env probe      -- per coverage host, assert the RUNTIME prerequisites
                                the mandatory live tiers need (GDK runtime up, an
                                Xbox identity signed in, a gamepad attached) via
                                tools\ci\live_env_probe.gd. Stage 0 validates the
                                settings; this validates the machine.
      5. GUT host runs       -- per coverage host:
                                  a. one-time `--headless --import` (marker file)
                                  b. `--headless -s res://addons/gut/gut_cmdln.gd
                                      -gdir=res://tests -gexit`
      6. PlayFab Multiplayer orchestrator -- C1 P0/P1 multi-client scenarios
      7. Bootstrap runners   -- `<host>\tests\bootstrap\*.gd` if present
      8. Aggregate           -- writes <OutDir>\run-summary.{json,md}

    The live and live-write tiers are ALWAYS ENABLED. There is no opt-out: every
    run sets LIVE_TESTS=1 and LIVE_WRITE_TESTS=1 for every Godot child, talks to
    live services, and mutates the configured PlayFab title. Runs therefore
    REQUIRE a dedicated sandbox title -- see the preflight check in Main.

    Because the tiers cannot be skipped, a test that cannot reach its live
    prerequisite must FAIL rather than degrade to `pending`. The shared GUT bases
    enforce this: `pending()` is overridden to fail whenever LIVE_TESTS=1, so a
    dormant test can no longer report green. Genuinely tolerated conditions
    (eventual-consistency settle timeouts, best-effort cleanup) call
    `pending_tolerated()` instead.

    Environment propagation goes through [System.Diagnostics.ProcessStartInfo]
    with UseShellExecute = $false. The orchestrator NEVER mutates $env:* in the
    parent shell. Async stdout/stderr drain is required (sync ReadToEnd() will
    deadlock once Godot floods stderr).

    GUT exits 0 even when zero tests are discovered. The orchestrator parses
    GUT's own summary block and asserts Tests > 0 per host; otherwise a
    misconfigured `-gdir` would silently be reported as green.

.PARAMETER SkipBuild
    Skips the CMake build stage. The doctest exe and the GUT mirrored copies
    must already exist from a prior build.

.PARAMETER SkipDoctest
    Skips the C++ doctest stage (stage 3); it is recorded as `skip` and does not
    abort the pipeline. The doctest is Godot-version independent, so CI builds
    that fan the Godot-dependent tiers across multiple engine versions run it
    once in the build job and pass -SkipDoctest to each per-Godot test leg.

.PARAMETER SkipGut
    Skips the GUT host stage (stage 4) entirely; each host is recorded as
    `skip` and does not abort the pipeline. Lets the PlayFab Multiplayer
    orchestrator (stage 5) run without first running the GUT live suites
    (useful when a flaky live GUT host would otherwise abort the orchestrator).

.PARAMETER SkipOrchestrator
    Skips the PlayFab Multiplayer orchestrator stage (stage 5). Use it to run
    the GUT/bootstrap suites without the multi-client live orchestrator.

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
    REQUIRED PlayFab title id forwarded to Godot children as PLAYFAB_TITLE_ID.
    The PlayFab test base applies it to ProjectSettings['playfab/runtime/title_id'].
    May also be supplied via the PLAYFAB_TITLE_ID environment variable. Must
    identify a dedicated sandbox title: the live-write tier is always on and
    mutates whatever title this names.

.PARAMETER PlayFabCustomId
    REQUIRED pre-existing PlayFab custom id forwarded to Godot children as
    PLAYFAB_CUSTOM_ID. Live custom-ID tests sign in with create_account=false.
    May also be supplied via the PLAYFAB_CUSTOM_ID environment variable.

.PARAMETER PlayFabMatchmakingQueue
    REQUIRED PlayFab matchmaking queue name forwarded to child processes as
    PLAYFAB_MULTIPLAYER_MATCH_QUEUE for Multiplayer live coverage. May also be
    supplied via the PLAYFAB_MULTIPLAYER_MATCH_QUEUE environment variable.

.PARAMETER GutTimeoutSec
    Per-host GUT and per-bootstrap-script timeout in seconds. Default: 600.

.PARAMETER VerboseOutput
    Streams child stdout/stderr to the host console as it arrives.

.NOTES
    The live and live-write tiers are ALWAYS ON and cannot be opted out of.
    Every run talks to live services and mutates the configured PlayFab title,
    so -PlayFabTitleId MUST name a dedicated sandbox title -- never a shared or
    production title id.

    Missing live configuration is a hard failure, not a skip: the preflight
    check below aborts the run before any stage executes when a required
    setting is absent. This is deliberate -- a run that cannot reach live
    services must never report green.

.OUTPUTS
    Writes <OutDir>\run-summary.json and <OutDir>\run-summary.md.
    Exits 0 on overall pass, 1 otherwise.
#>
[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [switch]$SkipDoctest,
    [switch]$SkipGut,
    [switch]$SkipOrchestrator,
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
$script:PlayFabMultiplayerOrchestratorRunner = Join-Path $script:RepoRoot 'tools\run_mp_orchestrator.ps1'

# GUT summary line regex. GUT (`addons/gut/summary.gd::_total_fmt`) renders each
# total as <label rpad 18><value lpad 5>. Values are integers, the literal
# "none" (when the count is zero), or for `Asserts` the form "<pass>/<total>"
# when at least one assert failed. Labels emitted only when non-zero are
# `Failing Tests`, `Risky/Pending`, `Orphans`. Wave 4 / docs may reference this
# regex to keep summary parsing in one place.
$script:GutSummaryRegex = '^\s*(?<label>Scripts|Tests|Passing Tests|Failing Tests|Risky/Pending|Asserts|Orphans|Time)\s+(?<value>\d+(?:/\d+)?|none|[\d\.]+s)\s*$'

# Runtime capabilities each coverage host must have available before its GUT
# suite runs. The live tiers are mandatory, so a host whose runtime prerequisites
# are absent must fail loudly in preflight rather than let hundreds of individual
# tests degrade to `pending`. Consumed by Invoke-LiveEnvironmentProbe; the
# capability names are parsed by tools\ci\live_env_probe.gd.
$script:HostLiveCapabilities = @{
    'tests\godot\gdk'       = @('gdk', 'xuser')
    'tests\godot\playfab'   = @('gdk', 'xuser', 'playfab')
    'tests\godot\gameinput' = @('gameinput', 'gamepad')
}

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
    # Scrub ambient LIVE_TESTS / LIVE_WRITE_TESTS so the tier flags can only
    # reach children via $EnvOverrides, which Main populates unconditionally.
    # Without this, an ambient LIVE_TESTS=0 in the developer's shell would
    # survive the overwrite order on some hosts and silently disable a tier
    # that is supposed to be mandatory.
    $psi.EnvironmentVariables.Remove('LIVE_TESTS')
    $psi.EnvironmentVariables.Remove('LIVE_WRITE_TESTS')
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
    # GUT registers `class_name` globals into `.godot/global_script_class_cache.cfg`.
    # On a cold cache (fresh checkout, or after Select-GutForGodotVersion swapped
    # the GUT version and invalidated the cache) a single `--headless --import`
    # scans resources but does not always complete class-name registration, so
    # GUT then aborts with "Some GUT class_names have not been imported". Two
    # successful import passes reliably settle the class cache. A pass can also
    # intermittently crash (access violation, exit 0xC0000005) mid-reimport of a
    # font/resource after a GUT swap; the reimport is incremental, so we retry a
    # crashed pass rather than failing. Require 2 clean passes within 4 attempts.
    $successPasses = 0
    $attempt = 0
    $lastFail = $null
    while ($successPasses -lt 2 -and $attempt -lt 4) {
        $attempt++
        $r = Invoke-ChildProcess -FileName $GodotExe -Arguments @('--headless', '--import') `
            -WorkingDirectory $HostRoot -EnvOverrides $ChildEnv -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
        if ($r.ExitCode -eq 0) {
            $successPasses++
            continue
        }
        $lastFail = $r
        if ($r.TimedOut) { break }
        Start-Sleep -Milliseconds 500
    }
    if ($successPasses -lt 2) {
        return [pscustomobject]@{
            ExitCode = $lastFail.ExitCode
            Output   = ($lastFail.Stdout + $lastFail.Stderr).Trim()
            TimedOut = $lastFail.TimedOut
        }
    }
    $markerDir = Split-Path -Parent $marker
    if (-not (Test-Path $markerDir)) {
        New-Item -ItemType Directory -Force -Path $markerDir | Out-Null
    }
    Set-Content -Path $marker -Value ("imported at " + (Get-Date -Format 'o')) -Encoding ASCII
    return $null
}

function Select-GutForGodotVersion {
    # Ensure each coverage host's `addons/gut` matches the Godot version under
    # test. The default CMake-mirrored GUT (`third_party/Gut`, 9.6.0) hard-requires
    # Godot 4.6+; Godot 4.5.x must use the 4.5-compatible mirror
    # (`third_party/Gut-4.5`, bitwes/Gut b366b70 = v9.5.0 + the #778 push_warning
    # fix). This runs once before the GUT host loop and (re)establishes the right
    # version deterministically, self-healing a tree left swapped by a prior run.
    #
    # Local dev only: it copies from the local submodule sources. In the
    # artifact-only CI path (no submodules checked out) the sources are absent, so
    # this is a no-op -- CI's run-test-tier action performs the swap itself.
    param(
        [Parameter(Mandatory = $true)][string]$GodotVersion,
        [Parameter(Mandatory = $true)][string[]]$HostList
    )

    $want45 = $GodotVersion -match '^4\.5(\.|-|$)'
    $srcRel = if ($want45) { 'third_party\Gut-4.5\addons\gut' } else { 'third_party\Gut\addons\gut' }
    $srcFull = Join-Path $script:RepoRoot $srcRel

    if (-not (Test-Path (Join-Path $srcFull 'gut_cmdln.gd'))) {
        # Artifact-only (CI) or missing submodule: trust the mirror already in place.
        return
    }

    foreach ($h in $HostList) {
        $dest = Join-Path (Join-Path $script:RepoRoot $h) 'addons\gut'
        Remove-Item $dest -Recurse -Force -ErrorAction SilentlyContinue
        Copy-Item $srcFull $dest -Recurse -Force
        # GUT registers class_name globals cached in `.godot`. Swapping the GUT
        # files invalidates that cache, and GUT aborts with "class_names have not
        # been imported" until a fresh `--headless --import`. Drop the orchestrator
        # import marker (and the stale class cache) so Ensure-HostImported re-imports.
        $hostGodot = Join-Path (Join-Path $script:RepoRoot $h) '.godot'
        Remove-Item (Join-Path $hostGodot 'orchestrator-imported') -Force -ErrorAction SilentlyContinue
        Remove-Item (Join-Path $hostGodot 'global_script_class_cache.cfg') -Force -ErrorAction SilentlyContinue
    }
    $label = if ($want45) { '4.5-compatible (Gut-4.5)' } else { 'default (Gut)' }
    Write-Host "   GUT: selected $label mirror for Godot $GodotVersion." -ForegroundColor Cyan
}

function Invoke-LiveEnvironmentProbe {
    # Runtime half of the live preflight. Assert-LiveConfiguration validates the
    # *settings*; this validates that the machine can actually exercise the
    # mandatory live tiers -- GDK runtime up, an Xbox identity signed in, a
    # gamepad attached. Without it those conditions surface only as hundreds of
    # individual in-test failures, which is slow and unreadable.
    param(
        [Parameter(Mandatory = $true)][string]$RelativeHost,
        [Parameter(Mandatory = $true)][string]$GodotExe,
        [Parameter(Mandatory = $true)][hashtable]$ChildEnv
    )
    $rec = New-StageRecord ("live-probe:" + ($RelativeHost -replace '\\','/'))
    $capabilities = $script:HostLiveCapabilities[$RelativeHost]
    if ($null -eq $capabilities -or $capabilities.Count -eq 0) {
        $rec.status  = 'skip'
        $rec.message = "No live capability requirements declared for '$RelativeHost'."
        return $rec
    }

    $hostRoot = Join-Path $script:RepoRoot $RelativeHost
    $sourceProbe = Join-Path $script:RepoRoot 'tools\ci\live_env_probe.gd'
    if (-not (Test-Path $sourceProbe)) {
        $rec.status  = 'fail'
        $rec.message = 'Live environment probe script not found at tools\ci\live_env_probe.gd.'
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

    # Staged at the host root as a temp script, mirroring how the CI load/smoke
    # step stages tools\ci\gdextension_load_check.gd, then removed.
    $tempProbe = Join-Path $hostRoot '_live_env_probe.gd'
    Copy-Item -Path $sourceProbe -Destination $tempProbe -Force
    try {
        $args = @('--headless', '-s', 'res://_live_env_probe.gd', '--') + $capabilities
        $r = Invoke-ChildProcess -FileName $GodotExe -Arguments $args -WorkingDirectory $hostRoot `
            -EnvOverrides $ChildEnv -TimeoutSec $GutTimeoutSec -Stream:$VerboseOutput
    } finally {
        Remove-Item -Path $tempProbe -Force -ErrorAction SilentlyContinue
    }

    $rec.exit_code = $r.ExitCode
    $rec.details   = ($r.Stdout + $r.Stderr).Trim()
    if ($r.TimedOut) {
        $rec.status  = 'fail'
        $rec.message = "Live environment probe timed out after $GutTimeoutSec s."
        return $rec
    }
    if ($r.ExitCode -ne 0) {
        $rec.status  = 'fail'
        $rec.message = "Live environment probe failed for $RelativeHost (requires: $($capabilities -join ', '))."
        return $rec
    }
    $rec.status  = 'pass'
    $rec.message = "All required capabilities available ($($capabilities -join ', '))."
    return $rec
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

function Get-MpP0P1ScenarioFilter {
    $scenarioRoot = Join-Path $script:RepoRoot 'tests\godot\mp_orchestrator\scenarios'
    if (-not (Test-Path $scenarioRoot)) { return '(?!)' }
    $ids = New-Object System.Collections.Generic.List[string]
    Get-ChildItem -Path $scenarioRoot -Recurse -Filter '*.gd' -File |
        Where-Object { $_.FullName -notmatch '\\_base\\' } |
        ForEach-Object {
            $text = Get-Content -Path $_.FullName -Raw -Encoding UTF8
            $idMatch = [regex]::Match($text, 'const\s+SCENARIO_ID:\s*String\s*=\s*"(?<id>[^"]+)"')
            $priorityMatch = [regex]::Match($text, 'const\s+PRIORITY:\s*String\s*=\s*"P[01]"')
            if ($idMatch.Success -and $priorityMatch.Success) {
                $id = $idMatch.Groups['id'].Value
                if (-not $id.StartsWith('_')) {
                    [void]$ids.Add($id)
                }
            }
        }
    if ($ids.Count -eq 0) { return '(?!)' }
    $escaped = $ids | Sort-Object -Unique | ForEach-Object { [regex]::Escape($_) }
    return '^(?:' + ($escaped -join '|') + ')$'
}

function Invoke-PlayFabMultiplayerOrchestrator {
    param(
        [Parameter(Mandatory = $true)][string]$GodotExe,
        [Parameter(Mandatory = $true)][hashtable]$ChildEnv,
        [Parameter(Mandatory = $true)][string[]]$HostList,
        [Parameter(Mandatory = $true)][string]$OutDirAbsolute
    )

    $rec = New-StageRecord 'playfab-multiplayer-orchestrator'
    # The live-write tier is mandatory, so this stage always runs. Every
    # scenario creates / updates / leaves lobbies (and optionally match
    # tickets) against the sandbox title validated by the preflight check.
    # The orchestrator is its own stage, independent of the GUT -Hosts filter
    # (use -SkipOrchestrator to exclude it, or -SkipGut to run it without the
    # GUT suites).
    if (-not (Test-Path $script:PlayFabMultiplayerOrchestratorRunner)) {
        $rec.status = 'fail'
        $rec.message = "PlayFab Multiplayer orchestrator runner not found at $script:PlayFabMultiplayerOrchestratorRunner."
        return $rec
    }

    $pwsh = Resolve-PwshExecutable
    $resultsDir = Join-Path $OutDirAbsolute 'mp-orchestrator'
    $filter = Get-MpP0P1ScenarioFilter
    $args = @(
        '-NoLogo', '-NoProfile', '-ExecutionPolicy', 'Bypass',
        '-File', $script:PlayFabMultiplayerOrchestratorRunner,
        '-Roles', 'host,guest,guest2,observer',
        '-Filter', $filter,
        '-ResultsDir', $resultsDir
    )

    $r = Invoke-ChildProcess -FileName $pwsh -Arguments $args -WorkingDirectory $script:RepoRoot `
        -EnvOverrides $ChildEnv -TimeoutSec ([Math]::Max($GutTimeoutSec * 3, 900)) -Stream:$VerboseOutput
    $rec.duration_ms = $r.DurationMs
    $rec.exit_code = $r.ExitCode
    $combined = ($r.Stdout + "`n" + $r.Stderr).Trim()
    $rec.details = if ($r.ExitCode -eq 0) { $null } else { $combined }

    if ($r.TimedOut) {
        $rec.status = 'fail'
        $rec.message = "PlayFab Multiplayer orchestrator timed out."
    } elseif ($r.ExitCode -ne 0) {
        $rec.status = 'fail'
        $rec.message = "PlayFab Multiplayer orchestrator failed (exit $($r.ExitCode))."
    } else {
        $resultJson = Join-Path $resultsDir 'mp-test-results.json'
        if (-not (Test-Path $resultJson)) {
            $rec.status = 'fail'
            $rec.message = "PlayFab Multiplayer orchestrator produced no mp-test-results.json at '$resultJson' (scenario scan may have failed or the runner exited early). Refusing to report PASS without scenario evidence."
            $rec.details = $combined
            return $rec
        }
        try {
            $mp = Get-Content -Path $resultJson -Raw -Encoding UTF8 | ConvertFrom-Json
        } catch {
            $rec.status = 'fail'
            $rec.message = "PlayFab Multiplayer orchestrator wrote mp-test-results.json but it failed to parse: $($_.Exception.Message)"
            $rec.details = $combined
            return $rec
        }
        if ($null -eq $mp.summary) {
            $rec.status = 'fail'
            $rec.message = "PlayFab Multiplayer orchestrator wrote mp-test-results.json with no .summary block."
            $rec.details = $combined
            return $rec
        }
        $rec.tests = [int]$mp.summary.total
        $rec.passing = [int]$mp.summary.passed
        $rec.failing = [int]$mp.summary.failed
        $rec.pending = [int]$mp.summary.skipped
        if ($rec.tests -le 0) {
            $rec.status = 'fail'
            $rec.message = "PlayFab Multiplayer orchestrator discovered no scenarios (total=0). Check the scenario filter or scan paths."
            $rec.details = $combined
            return $rec
        }
        if ($rec.failing -gt 0) {
            $rec.status = 'fail'
            $rec.message = "C1 P0/P1 scenarios: passed=$($rec.passing) failed=$($rec.failing) skipped=$($rec.pending)"
            $rec.details = $combined
            return $rec
        }
        $rec.status = 'pass'
        $rec.message = "C1 P0/P1 scenarios: passed=$($rec.passing) failed=$($rec.failing) skipped=$($rec.pending)"
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
        [Parameter(Mandatory = $true)][string]$PlayFabTitleIdUsed,
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
        live              = $true
        live_writes       = $true
        playfab_title_id  = $PlayFabTitleIdUsed
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
    [void]$mdLines.Add("- **Live**: True (always on)")
    [void]$mdLines.Add("- **Live writes**: True (always on)")
    [void]$mdLines.Add("- **PlayFab title id**: ``$PlayFabTitleIdUsed`` (sandbox)")
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

function Assert-LiveConfiguration {
    <#
        Hard-fail preflight for the mandatory live / live-write tiers.

        The live and live-write tiers are always on, so a run that lacks the
        configuration to reach live services cannot produce a meaningful
        result. Rather than let those tests degrade to `pending` (which would
        report green while verifying nothing), abort before any stage runs and
        name every missing setting at once.
    #>
    param(
        [string]$TitleId,
        [string]$CustomId,
        [string]$MatchmakingQueue
    )

    $missing = New-Object System.Collections.Generic.List[string]

    if ([string]::IsNullOrWhiteSpace($TitleId)) {
        [void]$missing.Add('PlayFab title id      -- pass -PlayFabTitleId <sandbox-title> or set PLAYFAB_TITLE_ID. Must be a DEDICATED SANDBOX title: the live-write tier mutates it.')
    }
    if ([string]::IsNullOrWhiteSpace($CustomId)) {
        [void]$missing.Add('PlayFab custom id     -- pass -PlayFabCustomId <custom-id> or set PLAYFAB_CUSTOM_ID. Live sign-in uses create_account=false, so the account must already exist (see tools\configure_playfab_test_title.ps1).')
    }
    if ([string]::IsNullOrWhiteSpace($MatchmakingQueue)) {
        [void]$missing.Add('PlayFab match queue   -- pass -PlayFabMatchmakingQueue <queue> or set PLAYFAB_MULTIPLAYER_MATCH_QUEUE. Required by the Multiplayer match scenarios (see tools\configure_playfab_test_title.ps1).')
    }

    if ($missing.Count -eq 0) { return }

    Write-Host ''
    Write-Host '=== PREFLIGHT FAILED: required live configuration is missing ===' -ForegroundColor Red
    Write-Host ''
    Write-Host 'The live and live-write test tiers are always enabled and cannot be skipped.' -ForegroundColor Red
    Write-Host 'A run without this configuration would verify nothing, so it is refused.' -ForegroundColor Red
    Write-Host ''
    foreach ($m in $missing) { Write-Host "  * $m" -ForegroundColor Yellow }
    Write-Host ''
    Write-Host 'Provision a sandbox title with tools\configure_playfab_test_title.ps1, then re-run.' -ForegroundColor Red
    Write-Host ''
    throw "Preflight failed: $($missing.Count) required live setting(s) missing. See the list above."
}

# ------------------------------------------------------------------------
# Main
# ------------------------------------------------------------------------

function Main {
    $startedAt = (Get-Date).ToUniversalTime()

    # Resolve live configuration from parameters first, then the environment.
    $effectivePlayFabTitleId = if (-not [string]::IsNullOrWhiteSpace($PlayFabTitleId)) {
        $PlayFabTitleId.Trim()
    } elseif (-not [string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable('PLAYFAB_TITLE_ID'))) {
        ([Environment]::GetEnvironmentVariable('PLAYFAB_TITLE_ID')).Trim()
    } else {
        ''
    }
    $effectivePlayFabCustomId = if (-not [string]::IsNullOrWhiteSpace($PlayFabCustomId)) {
        $PlayFabCustomId.Trim()
    } elseif (-not [string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable('PLAYFAB_CUSTOM_ID'))) {
        ([Environment]::GetEnvironmentVariable('PLAYFAB_CUSTOM_ID')).Trim()
    } else {
        ''
    }
    $effectivePlayFabMatchQueue = if (-not [string]::IsNullOrWhiteSpace($PlayFabMatchmakingQueue)) {
        $PlayFabMatchmakingQueue.Trim()
    } elseif (-not [string]::IsNullOrWhiteSpace([Environment]::GetEnvironmentVariable('PLAYFAB_MULTIPLAYER_MATCH_QUEUE'))) {
        ([Environment]::GetEnvironmentVariable('PLAYFAB_MULTIPLAYER_MATCH_QUEUE')).Trim()
    } else {
        ''
    }

    # 0. Preflight -- abort before any stage when live config is incomplete.
    Assert-LiveConfiguration -TitleId $effectivePlayFabTitleId `
        -CustomId $effectivePlayFabCustomId `
        -MatchmakingQueue $effectivePlayFabMatchQueue

    $godotExe  = Get-GodotExecutable
    $godotVer  = Get-GodotVersion -GodotExe $godotExe

    # The live tiers are mandatory: always forwarded, never conditional.
    $childEnv = @{
        LIVE_TESTS                      = '1'
        LIVE_WRITE_TESTS                = '1'
        PLAYFAB_TITLE_ID                = $effectivePlayFabTitleId
        PLAYFAB_CUSTOM_ID               = $effectivePlayFabCustomId
        PLAYFAB_MULTIPLAYER_MATCH_QUEUE = $effectivePlayFabMatchQueue
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
    Write-Host "                   SkipBuild = $SkipBuild   SkipDoctest = $SkipDoctest   SkipGut = $SkipGut   SkipOrchestrator = $SkipOrchestrator" -ForegroundColor Cyan
    Write-Host "                   PlayFabCustomId = set   PlayFabMatchmakingQueue = set" -ForegroundColor Cyan
    Write-Host "                   Hosts = $($hostList -join ', ')" -ForegroundColor Cyan
    Write-Host "                   ParseProjects = $(if ($parseProjectList.Count -gt 0) { $parseProjectList -join ', ' } else { 'all' })" -ForegroundColor Cyan
    Write-Host "                   ParseExcludeProjects = $(if ($parseExcludeProjectList.Count -gt 0) { $parseExcludeProjectList -join ', ' } else { 'none' })" -ForegroundColor Cyan
    Write-Host "                   OutDir= $outDirAbsolute" -ForegroundColor Cyan
    Write-Host "" -ForegroundColor Yellow
    Write-Host "[run_all_tests] === LIVE + LIVE-WRITE TIERS ENABLED (always on) ===" -ForegroundColor Yellow
    Write-Host "[run_all_tests] Active PlayFab title id: $effectivePlayFabTitleId" -ForegroundColor Yellow
    Write-Host "[run_all_tests] This MUST be a dedicated sandbox title. Never run against a shared or production title id." -ForegroundColor Yellow
    Write-Host ''

    $stages = New-Object System.Collections.Generic.List[object]
    $abort = $false

    # 1. Parse gate
    Write-Host '== [1/8] Parse gate (check_gd_scripts_headless.ps1) ==' -ForegroundColor Cyan
    $stage = Invoke-ParseGate -Projects $parseProjectList -ExcludeProjects $parseExcludeProjectList
    [void]$stages.Add($stage)
    Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
    if ($stage.status -ne 'pass') { $abort = $true }

    # 2. Build
    if (-not $abort) {
        Write-Host '== [2/8] CMake build (debug) ==' -ForegroundColor Cyan
        $stage = Invoke-Build
        [void]$stages.Add($stage)
        Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
        if ($stage.status -eq 'fail') { $abort = $true }
    } else {
        $skip = New-StageRecord 'cmake-build'; $skip.message = 'Skipped (upstream stage failed).'; [void]$stages.Add($skip)
    }

    # 3. C++ doctest
    if ($SkipDoctest) {
        Write-Host '== [3/8] C++ doctest (gdk_unit_tests.exe) ==' -ForegroundColor Cyan
        Write-Host '   SKIP: Skipped (-SkipDoctest).'
        $skip = New-StageRecord 'cpp-doctest'
        $skip.status = 'skip'
        $skip.message = 'Skipped (-SkipDoctest).'
        [void]$stages.Add($skip)
        Write-Host ''
    } elseif (-not $abort) {
        Write-Host '== [3/8] C++ doctest (gdk_unit_tests.exe) ==' -ForegroundColor Cyan
        $stage = Invoke-Doctest
        [void]$stages.Add($stage)
        Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
        if ($stage.status -ne 'pass') { $abort = $true }
    } else {
        $skip = New-StageRecord 'cpp-doctest'; $skip.message = 'Skipped (upstream stage failed).'; [void]$stages.Add($skip)
    }

    # 4. Live environment probe
    if ($SkipGut) {
        Write-Host '== [4/8] Live environment probe ==' -ForegroundColor Cyan
        Write-Host '   SKIP: Skipped (-SkipGut).'
        foreach ($h in $hostList) {
            $skip = New-StageRecord ("live-probe:" + ($h -replace '\\','/'))
            $skip.status = 'skip'
            $skip.message = 'Skipped (-SkipGut).'
            [void]$stages.Add($skip)
        }
        Write-Host ''
    } elseif (-not $abort) {
        Write-Host '== [4/8] Live environment probe ==' -ForegroundColor Cyan
        Select-GutForGodotVersion -GodotVersion $godotVer -HostList $hostList
        foreach ($h in $hostList) {
            Write-Host "  - host: $h"
            $stage = Invoke-LiveEnvironmentProbe -RelativeHost $h -GodotExe $godotExe -ChildEnv $childEnv
            [void]$stages.Add($stage)
            Write-Host "    $($stage.status.ToUpper()): $($stage.message)"
            if ($stage.status -eq 'fail') {
                if (-not [string]::IsNullOrWhiteSpace($stage.details)) {
                    foreach ($line in ($stage.details -split "`r?`n")) {
                        if ($line -match '\[live-probe\]') { Write-Host "      $line" -ForegroundColor Red }
                    }
                }
                $abort = $true
            }
        }
        Write-Host ''
    } else {
        Write-Host '== [4/8] Live environment probe ==' -ForegroundColor Cyan
        Write-Host '   SKIP: Skipped (upstream stage failed).'
        foreach ($h in $hostList) {
            $skip = New-StageRecord ("live-probe:" + ($h -replace '\\','/'))
            $skip.message = 'Skipped (upstream stage failed).'
            [void]$stages.Add($skip)
        }
        Write-Host ''
    }

    # 5. GUT runs
    if ($SkipGut) {
        Write-Host '== [5/8] GUT host runs ==' -ForegroundColor Cyan
        Write-Host '   SKIP: Skipped (-SkipGut).'
        foreach ($h in $hostList) {
            $skip = New-StageRecord ("gut:" + ($h -replace '\\','/'))
            $skip.status = 'skip'
            $skip.message = 'Skipped (-SkipGut).'
            [void]$stages.Add($skip)
        }
        Write-Host ''
    } elseif (-not $abort) {
        Write-Host '== [5/8] GUT host runs ==' -ForegroundColor Cyan
        Select-GutForGodotVersion -GodotVersion $godotVer -HostList $hostList
        foreach ($h in $hostList) {
            Write-Host "  - host: $h"
            $stage = Invoke-GutHost -RelativeHost $h -GodotExe $godotExe -ChildEnv $childEnv
            [void]$stages.Add($stage)
            Write-Host "    $($stage.status.ToUpper()): $($stage.message)"
            if ($stage.status -eq 'fail') { $abort = $true }
        }
        Write-Host ''
    } else {
        Write-Host '== [5/8] GUT host runs ==' -ForegroundColor Cyan
        Write-Host '   SKIP: Skipped (upstream stage failed).'
        foreach ($h in $hostList) {
            $skip = New-StageRecord ("gut:" + ($h -replace '\\','/'))
            $skip.message = 'Skipped (upstream stage failed).'
            [void]$stages.Add($skip)
        }
        Write-Host ''
    }

    # 6. PlayFab Multiplayer orchestrator
    if ($SkipOrchestrator) {
        Write-Host '== [6/8] PlayFab Multiplayer orchestrator (C1 P0/P1) ==' -ForegroundColor Cyan
        $skip = New-StageRecord 'playfab-multiplayer-orchestrator'
        $skip.status = 'skip'
        $skip.message = 'Skipped (-SkipOrchestrator).'
        [void]$stages.Add($skip)
        Write-Host "   SKIP: $($skip.message)`n"
    } elseif (-not $abort) {
        Write-Host '== [6/8] PlayFab Multiplayer orchestrator (C1 P0/P1) ==' -ForegroundColor Cyan
        $stage = Invoke-PlayFabMultiplayerOrchestrator -GodotExe $godotExe -ChildEnv $childEnv -HostList $hostList -OutDirAbsolute $outDirAbsolute
        [void]$stages.Add($stage)
        Write-Host "   $($stage.status.ToUpper()): $($stage.message)`n"
        if ($stage.status -eq 'fail') { $abort = $true }
    } else {
        Write-Host '== [6/8] PlayFab Multiplayer orchestrator (C1 P0/P1) ==' -ForegroundColor Cyan
        Write-Host "   SKIP: Skipped (upstream stage failed).`n"
        $skip = New-StageRecord 'playfab-multiplayer-orchestrator'
        $skip.message = 'Skipped (upstream stage failed).'
        [void]$stages.Add($skip)
    }

    # 7. Bootstrap mini-runners (run even if GUT failed? spec says abort on failure;
    # we honor the abort to keep the pipeline simple.)
    if (-not $abort) {
        Write-Host '== [7/8] Bootstrap mini-runners ==' -ForegroundColor Cyan
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
        Write-Host '== [7/8] Bootstrap mini-runners ==' -ForegroundColor Cyan
        Write-Host "   SKIP: Skipped (upstream stage failed).`n"
        $skip = New-StageRecord 'bootstrap'
        $skip.message = 'Skipped (upstream stage failed).'
        [void]$stages.Add($skip)
    }

    # 8. Aggregate
    Write-Host '== [8/8] Aggregate run summary ==' -ForegroundColor Cyan
    $finishedAt = (Get-Date).ToUniversalTime()
    $overall = if ($stages | Where-Object { $_.status -eq 'fail' }) { 'fail' } else { 'pass' }
    $written = Write-RunSummary -Stages $stages -OutDirAbsolute $outDirAbsolute `
        -OverallStatus $overall -StartedAtUtc $startedAt -FinishedAtUtc $finishedAt `
        -PlayFabTitleIdUsed $effectivePlayFabTitleId -GodotVersion $godotVer
    Write-Host "   wrote $($written.JsonPath)"
    Write-Host "   wrote $($written.MdPath)"
    Write-Host ''
    Write-Host "Overall: $overall" -ForegroundColor $(if ($overall -eq 'pass') { 'Green' } else { 'Red' })
    Write-Host "Summary: $($written.MdPath)"

    if ($overall -eq 'pass') { exit 0 } else { exit 1 }
}

Main
