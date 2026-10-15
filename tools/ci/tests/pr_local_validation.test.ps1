#Requires -Version 7.0
<#
.SYNOPSIS
    Offline contract tests for the PR-local validation tooling.

.DESCRIPTION
    Runs with no network, no Godot, no GDK, and no PlayFab. It covers the two
    modules that decide what a validation run is allowed to claim:

      tools\ci\GodotAcquisition.psm1  - supply-chain refusal rules
      tools\ci\PrLocalValidation.psm1 - verdict honesty and Markdown safety

    It also executes the wrapper's build/reporting block with simulated tools,
    so scoped success and retained failure evidence need no live-service access.

    Deliberately no new test framework: this is plain PowerShell so it can run
    inside the existing ci-lint job, which already has pwsh but no Pester.

    Exit code 0 when every test passes, 1 otherwise.
#>
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$here = Split-Path -Parent $PSCommandPath
$ciRoot = Split-Path -Parent $here

Import-Module (Join-Path $ciRoot 'GodotAcquisition.psm1') -Force
Import-Module (Join-Path $ciRoot 'PrLocalValidation.psm1') -Force

$script:Passed = 0
$script:Failed = 0

function Test-Case {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [Parameter(Mandatory = $true)][scriptblock]$Body
    )
    try {
        & $Body
        $script:Passed++
        Write-Host "  PASS  $Name" -ForegroundColor Green
    } catch {
        $script:Failed++
        Write-Host "  FAIL  $Name" -ForegroundColor Red
        Write-Host "        $($_.Exception.Message)" -ForegroundColor Red
    }
}

function Assert-True {
    param([Parameter(Mandatory = $true)][bool]$Condition, [string]$Message = 'Expected condition to be true.')
    if (-not $Condition) { throw $Message }
}

function Assert-Equal {
    param($Expected, $Actual, [string]$Message)
    if ($Expected -ne $Actual) {
        throw ("$Message Expected '<{0}>' but got '<{1}>'." -f $Expected, $Actual)
    }
}

function Assert-Throws {
    param(
        [Parameter(Mandatory = $true)][scriptblock]$Body,
        [Parameter(Mandatory = $true)][string]$MatchPattern
    )
    try {
        & $Body
    } catch {
        if ($_.Exception.Message -notmatch $MatchPattern) {
            throw "Threw, but the message did not match '$MatchPattern': $($_.Exception.Message)"
        }
        return
    }
    throw "Expected a terminating error matching '$MatchPattern', but none was thrown."
}

function New-Stage {
    param(
        [Parameter(Mandatory = $true)][string]$Name,
        [string]$Status = 'pass',
        $Tests = 10, $Passing = 10, $Failing = 0, $Pending = 0
    )
    return [pscustomobject]@{
        name = $Name; status = $Status; duration_ms = 1000; exit_code = 0
        tests = $Tests; passing = $Passing; failing = $Failing; pending = $Pending
    }
}

function New-Summary {
    param(
        [string]$Overall = 'pass',
        [bool]$Live = $true,
        [bool]$LiveWrites = $true,
        $Stages,
        # Appended to $Stages so the many cases that only care about GUT hosts
        # do not each have to restate a realistic bootstrap set. Pass @() to
        # model a run whose bootstrap runners never executed.
        $BootstrapStages
    )
    if ($null -eq $BootstrapStages) {
        $BootstrapStages = @(
            (New-Stage -Name 'bootstrap:tests/godot/gdk:run_gdk_bootstrap'),
            (New-Stage -Name 'bootstrap:tests/godot/playfab:run_playfab_bootstrap'),
            (New-Stage -Name 'bootstrap:tests/godot/gameinput:run_gameinput_bootstrap')
        )
    }
    if ($null -eq $Stages) {
        $Stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
    }
    return [pscustomobject]@{
        overall_status = $Overall
        started_at = '2026-01-01T00:00:00Z'
        finished_at = '2026-01-01T01:00:00Z'
        total_duration_ms = 3600000
        live = $Live
        live_writes = $LiveWrites
        godot_version = '4.7.1-stable'
        stages = @($Stages) + @($BootstrapStages)
    }
}

# --------------------------------------------------------------------------
Write-Host 'GodotAcquisition: supply-chain refusals'
# --------------------------------------------------------------------------

$fixtureRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("pr-local-validation-tests-" + [guid]::NewGuid().ToString('n'))
New-Item -ItemType Directory -Force -Path $fixtureRoot | Out-Null

try {
    $goodHash = ('a' * 128)
    $manifestPath = Join-Path $fixtureRoot 'godot-versions.json'
    @{
        default   = '4.7.1-stable'
        supported = @('4.7.1-stable', '4.6.1-stable')
        sha512    = @{ '4.7.1-stable' = $goodHash }
    } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $manifestPath -Encoding utf8

    Test-Case 'manifest parses default/supported/sha512' {
        $m = Read-GodotManifest -ManifestPath $manifestPath
        Assert-Equal '4.7.1-stable' $m.Default 'Default version mismatch.'
        Assert-Equal 2 @($m.Supported).Count 'Supported count mismatch.'
    }

    Test-Case 'manifest with a default outside supported is rejected' {
        $bad = Join-Path $fixtureRoot 'bad-default.json'
        @{ default = '4.9.9-stable'; supported = @('4.7.1-stable'); sha512 = @{} } |
            ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $bad -Encoding utf8
        Assert-Throws -MatchPattern "is not in its own 'supported' list" -Body { Read-GodotManifest -ManifestPath $bad }
    }

    Test-Case 'version strings outside the pinned shape are rejected' {
        Assert-True (Test-GodotVersionString '4.7.1-stable') 'Valid version rejected.'
        Assert-True (Test-GodotVersionString '4.6-stable') 'Valid two-part version rejected.'
        Assert-True (-not (Test-GodotVersionString '../../etc/passwd')) 'Path traversal accepted.'
        Assert-True (-not (Test-GodotVersionString '4.7.1-stable; rm -rf /')) 'Command injection accepted.'
        Assert-True (-not (Test-GodotVersionString '4.7.1')) 'Version without a flavor accepted.'
    }

    Test-Case 'an unpinned version refuses to resolve' {
        Assert-Throws -MatchPattern 'No pinned SHA-512' -Body {
            Get-GodotPin -Version '4.6.1-stable' -ManifestPath $manifestPath
        }
    }

    Test-Case 'a malformed pin refuses to resolve' {
        $bad = Join-Path $fixtureRoot 'bad-hash.json'
        @{
            default = '4.7.1-stable'; supported = @('4.7.1-stable')
            sha512 = @{ '4.7.1-stable' = 'not-a-hash' }
        } | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $bad -Encoding utf8
        Assert-Throws -MatchPattern 'SHA-512' -Body { Get-GodotPin -Version '4.7.1-stable' -ManifestPath $bad }
    }

    Test-Case 'the pinned URL points at the official godot-builds release' {
        $pin = Get-GodotPin -Version '4.7.1-stable' -ManifestPath $manifestPath
        Assert-True ($pin.Url -eq "https://github.com/godotengine/godot-builds/releases/download/4.7.1-stable/Godot_v4.7.1-stable_win64.exe.zip") "Unexpected URL '$($pin.Url)'."
        Assert-Equal 'Godot_v4.7.1-stable_win64.exe.zip' $pin.Asset 'Unexpected asset name.'
        Assert-Equal $goodHash $pin.Sha512 'The pinned hash was not carried through.'
    }

    Test-Case 'a hash mismatch on a downloaded file is fatal' {
        $f = Join-Path $fixtureRoot 'payload.bin'
        Set-Content -LiteralPath $f -Value 'contents' -NoNewline -Encoding utf8
        Assert-Throws -MatchPattern 'SHA-512 mismatch' -Body { Assert-FileSha512 -Path $f -Expected $goodHash }
    }

    Test-Case 'a matching hash is accepted' {
        $f = Join-Path $fixtureRoot 'payload.bin'
        $actual = (Get-FileHash -LiteralPath $f -Algorithm SHA512).Hash.ToLowerInvariant()
        Assert-FileSha512 -Path $f -Expected $actual | Out-Null
    }

    Test-Case 'the reported engine version must identify the requested tag' {
        Assert-True (Test-GodotReportedVersion -Version '4.7.1-stable' -Reported '4.7.1.stable.official.abcdef123') 'Matching report rejected.'
        Assert-True (-not (Test-GodotReportedVersion -Version '4.7.1-stable' -Reported '4.6.1.stable.official')) 'Mismatched report accepted.'
        Assert-True (-not (Test-GodotReportedVersion -Version '4.7.1-stable' -Reported '')) 'Empty report accepted.'
    }

    Test-Case 'console flavor resolves the console executable name' {
        Assert-Equal 'Godot_v4.7.1-stable_win64_console.exe' (Get-GodotExecutableName -Version '4.7.1-stable' -Flavor 'console') 'Console name mismatch.'
        Assert-Equal 'Godot_v4.7.1-stable_win64.exe' (Get-GodotExecutableName -Version '4.7.1-stable' -Flavor 'editor') 'Editor name mismatch.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: verdict honesty'
    # --------------------------------------------------------------------------

    Test-Case 'a fully green live-write leg passes' {
        $v = Get-EngineLegVerdict -Summary (New-Summary) -RequireOrchestrator
        Assert-Equal 'pass' $v.Status "Unexpected gaps: $($v.Gaps -join '; ') reasons: $($v.Reasons -join '; ')"
        # Totals count the GUT hosts only (3 x 10). Orchestrator scenarios are
        # reported as their own stage, not folded into the GUT test count.
        Assert-Equal 30 $v.Tests 'Test totals were not aggregated across the GUT hosts.'
        Assert-Equal 30 $v.Passing 'Passing totals were not aggregated across the GUT hosts.'
    }

    Test-Case 'a missing run-summary is an error, not a pass' {
        $v = Get-EngineLegVerdict -Summary $null
        Assert-Equal 'error' $v.Status 'A missing summary must not be interpreted.'
    }

    Test-Case 'a summary missing required fields is an error' {
        $v = Get-EngineLegVerdict -Summary ([pscustomobject]@{ overall_status = 'pass'; stages = @() })
        Assert-Equal 'error' $v.Status 'An unparseable summary must not be interpreted.'
    }

    Test-Case 'overall_status fail produces fail' {
        $v = Get-EngineLegVerdict -Summary (New-Summary -Overall 'fail')
        Assert-Equal 'fail' $v.Status 'A failing run must fail.'
    }

    Test-Case 'a leg that did not run live cannot claim live coverage' {
        $v = Get-EngineLegVerdict -Summary (New-Summary -Live $false)
        Assert-Equal 'error' $v.Status 'A non-live run must not be reported as live.'
    }

    Test-Case 'a leg without live writes cannot claim live-write coverage' {
        $v = Get-EngineLegVerdict -Summary (New-Summary -LiveWrites $false)
        Assert-Equal 'error' $v.Status 'A read-only run must not be reported as a live-write run.'
    }

    Test-Case 'a missing GUT host is incomplete, not pass' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'A missing host must downgrade the verdict.'
        Assert-True ([bool](@($v.Gaps) -match 'gameinput')) 'The gap should name the missing host.'
    }

    Test-Case 'a skipped GUT host is incomplete' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Status 'skip' -Tests 0 -Passing 0),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'A skipped host must downgrade the verdict.'
    }

    Test-Case 'a GUT host that discovered no tests is incomplete' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Tests 0 -Passing 0),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'Zero discovered tests must downgrade the verdict.'
    }

    Test-Case 'a GUT host where everything was skipped is incomplete' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Tests 10 -Passing 0 -Pending 10),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'An all-pending host must downgrade the verdict.'
    }

    Test-Case 'an orchestrator that passed no scenarios is incomplete' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 0 -Passing 0)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'A zero-scenario orchestrator run must downgrade the verdict.'
    }

    # The orchestrator leaves every per-stage counter null even after running
    # dozens of scenarios; its real tallies only exist in mp-test-results.json.
    # Reading the null counter as "passed nothing" produced a false coverage
    # gap on a real PR comment, so these three cases pin the precedence.
    Test-Case 'orchestrator scenario counts come from mp-test-results, not the null stage counters' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests $null -Passing $null)
        )
        $mp = [pscustomobject]@{ summary = [pscustomobject]@{ total = 69; passed = 69; failed = 0; skipped = 0 } }
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -OrchestratorResult $mp -RequireOrchestrator
        Assert-Equal 'pass' $v.Status "A fully passing orchestrator must not be a gap. Gaps: $($v.Gaps -join '; ')"
        Assert-Equal 69 $v.MpTotal 'The scenario total was not carried through.'
        Assert-Equal 69 $v.MpPassed 'The passed scenario count was not carried through.'
        Assert-Equal 0 $v.MpFailed 'The failed scenario count was not carried through.'
    }

    Test-Case 'failed scenarios are a failure, and their counts still surface' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Status 'fail' -Tests $null -Passing $null)
        )
        $mp = [pscustomobject]@{ summary = [pscustomobject]@{ total = 69; passed = 59; failed = 10; skipped = 0 } }
        $v = Get-EngineLegVerdict -Summary (New-Summary -Overall 'fail' -Stages $stages) -OrchestratorResult $mp -RequireOrchestrator
        Assert-Equal 'fail' $v.Status 'Failing scenarios must fail the leg.'
        Assert-Equal 59 $v.MpPassed 'The passed scenario count was not carried through.'
        Assert-Equal 10 $v.MpFailed 'The failed scenario count was not carried through.'
        Assert-True (-not ([bool](@($v.Gaps) -match 'passed no scenarios'))) 'A run that passed 59 scenarios must not claim it passed none.'
    }

    Test-Case 'an orchestrator with no counts anywhere is incomplete, not pass' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests $null -Passing $null)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'Unverifiable orchestrator coverage must not pass.'
        Assert-True ([bool](@($v.Gaps) -match 'no scenario counts')) 'The gap should say the counts were missing.'
    }

    Test-Case 'a skipped parse gate is incomplete' {
        $stages = @(
            (New-Stage -Name 'parse-gate' -Status 'skip' -Tests 0 -Passing 0),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'A skipped parse gate must downgrade the verdict.'
    }

    # run_all_tests.ps1 derives overall_status from failures only, so a stage
    # that never ran leaves the run green. These cases pin the two stages the
    # report claims were exercised but nothing else was checking.
    Test-Case 'a missing cpp-doctest stage is incomplete on the first leg' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'incomplete' $v.Status 'A missing native suite must downgrade the verdict.'
        Assert-True ([bool](@($v.Gaps) -match 'cpp-doctest')) 'The gap should name the native stage.'
    }

    Test-Case 'a skipped cpp-doctest stage is incomplete on the first leg' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest' -Status 'skip' -Tests 0 -Passing 0),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'incomplete' $v.Status 'A skipped native suite must downgrade the verdict.'
    }

    Test-Case 'a failed cpp-doctest stage fails the leg' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest' -Status 'fail'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Overall 'fail' -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'fail' $v.Status 'A failing native suite must fail the leg.'
    }

    # The wrapper appends -SkipDoctest to every leg after the first on purpose,
    # so only the first leg may assert it. A later leg must not be penalised.
    Test-Case 'a later leg is not penalised for the deliberately skipped doctest' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest' -Status 'skip' -Tests 0 -Passing 0),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator
        Assert-Equal 'pass' $v.Status "Later legs intentionally skip the native suite. Gaps: $($v.Gaps -join '; ')"
    }

    Test-Case 'a host with no bootstrap stages at all is incomplete' {
        $v = Get-EngineLegVerdict -Summary (New-Summary -BootstrapStages @()) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'Absent bootstrap coverage must downgrade the verdict.'
        Assert-True ([bool](@($v.Gaps) -match 'bootstrap:tests/godot/gdk')) 'The gap should name the host.'
    }

    # An absent or empty tests\bootstrap\ directory collapses to one 'skip'
    # record, which overall_status ignores entirely.
    Test-Case 'a host whose bootstrap runners were all skipped is incomplete' {
        $boot = @(
            (New-Stage -Name 'bootstrap:tests/godot/gdk' -Status 'skip' -Tests 0 -Passing 0),
            (New-Stage -Name 'bootstrap:tests/godot/playfab:run_playfab_bootstrap'),
            (New-Stage -Name 'bootstrap:tests/godot/gameinput:run_gameinput_bootstrap')
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -BootstrapStages $boot) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'An all-skipped bootstrap host must downgrade the verdict.'
        Assert-True ([bool](@($v.Gaps) -match 'ran no bootstrap suites')) 'The gap should say nothing ran.'
    }

    Test-Case 'a failed bootstrap runner fails the leg' {
        $boot = @(
            (New-Stage -Name 'bootstrap:tests/godot/gdk:run_gdk_bootstrap' -Status 'fail'),
            (New-Stage -Name 'bootstrap:tests/godot/playfab:run_playfab_bootstrap'),
            (New-Stage -Name 'bootstrap:tests/godot/gameinput:run_gameinput_bootstrap')
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Overall 'fail' -BootstrapStages $boot) -RequireOrchestrator
        Assert-Equal 'fail' $v.Status 'A failing bootstrap runner must fail the leg.'
        Assert-True ([bool](@($v.Reasons) -match 'run_gdk_bootstrap')) 'The reason should name the runner.'
    }

    # 'bootstrap:tests/godot/gdk2' must not satisfy a requirement for
    # 'bootstrap:tests/godot/gdk'; the match is on an exact name or a ':' boundary.
    Test-Case 'bootstrap host matching does not collide on a name prefix' {
        $boot = @(
            (New-Stage -Name 'bootstrap:tests/godot/gdk2:run_other'),
            (New-Stage -Name 'bootstrap:tests/godot/playfab:run_playfab_bootstrap'),
            (New-Stage -Name 'bootstrap:tests/godot/gameinput:run_gameinput_bootstrap')
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -BootstrapStages $boot) -RequireOrchestrator
        Assert-Equal 'incomplete' $v.Status 'A similarly named host must not satisfy the requirement.'
        Assert-True ([bool](@($v.Gaps) -match 'bootstrap:tests/godot/gdk did not run')) 'The gap should name the genuinely missing host.'
    }

    # run_all_tests.ps1's exit code is the one signal the summary file cannot
    # carry. A leg that crashed, was cancelled, or failed after writing an
    # otherwise green summary must never settle as 'pass'.
    Test-Case 'a nonzero orchestrator exit code fails an otherwise green leg' {
        $v = Get-EngineLegVerdict -Summary (New-Summary) -RequireOrchestrator -OrchestratorExitCode 1
        Assert-Equal 'fail' $v.Status 'A nonzero orchestrator exit must fail the leg.'
        Assert-True ([bool](@($v.Reasons) -match 'exited 1')) 'The reason should name the exit code.'
    }

    Test-Case 'a nonzero exit code upgrades an incomplete leg to fail' {
        $v = Get-EngineLegVerdict -Summary (New-Summary -BootstrapStages @()) -RequireOrchestrator -OrchestratorExitCode 2
        Assert-Equal 'fail' $v.Status 'An exit-code failure must outrank incomplete.'
        Assert-True ([bool](@($v.Reasons) -match 'exited 2')) 'The reason should name the exit code.'
    }

    Test-Case 'a nonzero exit code cannot downgrade an error verdict' {
        $v = Get-EngineLegVerdict -Summary (New-Summary -Live $false) -RequireOrchestrator -OrchestratorExitCode 2
        Assert-Equal 'error' $v.Status 'fail must not mask a worse error verdict.'
    }

    Test-Case 'a zero orchestrator exit code leaves a green leg green' {
        $v = Get-EngineLegVerdict -Summary (New-Summary) -RequireOrchestrator -OrchestratorExitCode 0
        Assert-Equal 'pass' $v.Status "Unexpected reasons: $($v.Reasons -join '; ')"
    }

    Test-Case 'Get-SummaryStagesByPrefix tolerates a null or stage-less summary' {
        Assert-Equal 0 (@(Get-SummaryStagesByPrefix -Summary $null -Prefix 'bootstrap').Count) 'A null summary must yield no stages.'
        $bare = [pscustomobject]@{ overall_status = 'pass' }
        Assert-Equal 0 (@(Get-SummaryStagesByPrefix -Summary $bare -Prefix 'bootstrap').Count) 'A summary without stages must yield no stages.'
    }

    Test-Case 'worst status wins and maps to the documented exit codes' {
        Assert-Equal 'error' (Get-WorstStatus @('pass', 'incomplete', 'error', 'fail')) 'error must dominate.'
        Assert-Equal 'fail' (Get-WorstStatus @('pass', 'incomplete', 'fail')) 'fail must beat incomplete.'
        Assert-Equal 'incomplete' (Get-WorstStatus @('pass', 'incomplete')) 'incomplete must beat pass.'
        Assert-Equal 'pass' (Get-WorstStatus @('pass', 'pass')) 'all-pass must be pass.'
        Assert-Equal 'error' (Get-WorstStatus @()) 'An empty set must not report success.'
        Assert-Equal 0 (Get-ExitCodeForStatus 'pass') 'pass exit code.'
        Assert-Equal 2 (Get-ExitCodeForStatus 'incomplete') 'incomplete exit code.'
        Assert-Equal 1 (Get-ExitCodeForStatus 'fail') 'fail exit code.'
        Assert-Equal 1 (Get-ExitCodeForStatus 'error') 'error exit code.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: identity proof'
    # --------------------------------------------------------------------------

    Test-Case 'the GRDK edition is read from the resolved header' {
        $header = "/* generated */`n#define _GRDK_EDITION            260402`n#pragma comment(linker, `"/include:_XBLD_GRDK_EDITION_260402`")`n"
        Assert-Equal '260402' (Get-GrdkEdition -HeaderText $header) 'Edition not parsed.'
    }

    Test-Case 'a header without the edition macro yields null, never a fallback' {
        Assert-Equal $null (Get-GrdkEdition -HeaderText "#define SOMETHING_ELSE 1`n") 'Absent macro must yield null.'
        Assert-Equal $null (Get-GrdkEdition -HeaderText '') 'Empty header must yield null.'
        # The linker pragma alone must not be mistaken for the definition.
        Assert-Equal $null (Get-GrdkEdition -HeaderText '#pragma comment(linker, "/include:_XBLD_GRDK_EDITION_260402")') 'Pragma must not be parsed as the macro.'
    }

    Test-Case 'the ms-gdk package version is read from vcpkg spdx metadata' {
        $spdx = [pscustomobject]@{ packages = @(
            [pscustomobject]@{ name = 'ms-gdk'; versionInfo = '2604.2.7849' },
            [pscustomobject]@{ name = 'ms-gdk:x64-windows'; versionInfo = 'abcdef' }
        ) }
        Assert-Equal '2604.2.7849' (Get-MsGdkPackageVersion -Spdx $spdx) 'Package version not parsed.'
        Assert-Equal $null (Get-MsGdkPackageVersion -Spdx $null) 'Null spdx must yield null.'
        Assert-Equal $null (Get-MsGdkPackageVersion -Spdx ([pscustomobject]@{ packages = @() })) 'Empty spdx must yield null.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: sandbox handling'
    # --------------------------------------------------------------------------

    Test-Case 'only documented sandbox id shapes are accepted' {
        Assert-True (Test-SandboxId 'RETAIL') 'RETAIL rejected.'
        Assert-True (Test-SandboxId 'lykhvw.0') 'Valid sandbox rejected.'
        Assert-True (-not (Test-SandboxId 'lykhvw.0 & calc.exe')) 'Command injection accepted.'
        Assert-True (-not (Test-SandboxId '/noApps')) 'Switch-like value accepted.'
        Assert-True (-not (Test-SandboxId '')) 'Empty value accepted.'
    }

    Test-Case 'the current sandbox is parsed from tool output, or reported unknown' {
        Assert-Equal 'lykhvw.0' (Get-SandboxIdFromOutput -Output "Current sandbox is lykhvw.0`n") 'Sandbox not parsed.'
        Assert-Equal 'RETAIL' (Get-SandboxIdFromOutput -Output 'RETAIL') 'RETAIL not parsed.'
        Assert-Equal $null (Get-SandboxIdFromOutput -Output 'access denied') 'Unrecognized output must be null, not a guess.'
        Assert-Equal $null (Get-SandboxIdFromOutput -Output '') 'Empty output must be null.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: Markdown safety'
    # --------------------------------------------------------------------------

    Test-Case 'pipes and newlines cannot break out of a table cell' {
        $cell = ConvertTo-MarkdownCell "a | b`nc"
        Assert-True ($cell -notmatch '(?<!\\)\|') "Unescaped pipe survived: '$cell'."
        Assert-True ($cell -notmatch "`n") 'Newline survived into a table cell.'
    }

    Test-Case 'a fenced block is always terminated, even when truncated' {
        $long = ('x' * 10000)
        $block = Format-FencedBlock -Text $long -MaxChars 100
        $fences = ([regex]::Matches($block, '(?m)^`{3,}')).Count
        Assert-Equal 2 $fences "Expected exactly two fences, got $fences."
        Assert-True ($block -like '*(truncated)*') 'Truncation was not disclosed.'
    }

    Test-Case 'backtick runs inside content cannot close the fence early' {
        $block = Format-FencedBlock -Text "before ``````` after"
        $fences = ([regex]::Matches($block, '(?m)^`{3,}')).Count
        Assert-Equal 2 $fences "Content backticks closed the fence: $block"
        $open = [regex]::Match($block, '(?m)^(`{3,})').Groups[1].Value
        Assert-True ($open.Length -ge 4) 'The fence was not widened past the content backtick run.'
    }

    Test-Case 'a language hint is only emitted when it is a plain token' {
        # NB: -like cannot be used here because the backtick is the wildcard
        # escape character, so a fence literal never matches.
        Assert-True ((Format-FencedBlock -Text 'hi' -Language 'text').StartsWith('```text')) 'Valid language hint dropped.'
        Assert-True (-not (Format-FencedBlock -Text 'hi' -Language "text`ninjected").Contains('injected')) 'Injected language hint accepted.'
        Assert-True ((Format-FencedBlock -Text 'hi').StartsWith('```' + "`n")) 'An absent language hint should leave the info string empty.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: report and run naming'
    # --------------------------------------------------------------------------

    $manifestObject = [pscustomobject]@{
        schema_version = 1
        pull_request = 202
        repository = 'microsoft/XBOX-Godot-Sample'
        head_sha = ('f' * 40)
        status = 'incomplete'
        started_at = '2026-01-01T00:00:00Z'
        finished_at = '2026-01-01T02:00:00Z'
        gdk_edition = '260402'
        ms_gdk_version = '2604.2.7849'
        gdk_declared_default = '2604.2.7849'
        engines_selected = @('4.7.1-stable', '4.6.1-stable')
        engines_skipped = @()
        engines_not_run = @()
        playfab_title_id = '10D176'
        sandbox_id = 'lykhvw.0'
        sandbox_restored_to = 'RETAIL'
        release_build_status = 'pass'
        engines = @(
            [pscustomobject]@{ version = '4.7.1-stable'; status = 'pass'; tests = 30; passing = 30; failing = 0; pending = 0; duration_ms = 61000; reasons = @(); gaps = @() },
            [pscustomobject]@{ version = '4.6.1-stable'; status = 'incomplete'; tests = 20; passing = 20; failing = 0; pending = 0; duration_ms = 55000; reasons = @(); gaps = @('gut:tests/godot/gameinput was skipped.') }
        )
        reasons = @()
        gaps = @()
    }

    Test-Case 'the comment states the head commit, SDK identity, and gaps' {
        $body = Format-ValidationComment -Manifest $manifestObject
        Assert-True ($body -like "*$('f' * 40)*") 'The head commit is missing from the report.'
        Assert-True ($body -like '*260402*') 'The GDK edition is missing from the report.'
        Assert-True ($body -like '*2604.2.7849*') 'The ms-gdk version is missing from the report.'
        Assert-True ($body -like '*INCOMPLETE*') 'A non-green run must say so in the heading.'
        Assert-True ($body -like '*Coverage gaps*') 'Coverage gaps were not reported.'
        Assert-True ($body -like '*Not covered by this run*') 'The scope caveats are missing.'
        Assert-True ($body -like '*pr-local-validation*') 'The marker comment is missing.'
    }

    Test-Case 'the comment table has one row per engine' {
        $body = Format-ValidationComment -Manifest $manifestObject
        $rows = @($body -split "`n" | Where-Object { $_ -match '^\| `4\.' })
        Assert-Equal 2 $rows.Count "Expected two engine rows, got $($rows.Count)."
    }

    Test-Case 'the comment limits its claim to the exact commit and selected scope' {
        $body = Format-ValidationComment -Manifest $manifestObject
        Assert-True ($body -like '*Results apply only to the exact commit*') 'The result must be scoped.'
        Assert-True (-not ($body -like '*on every Godot version*')) 'The report must not infer full-matrix success.'
    }

    Test-Case 'a narrowed comment drops the full-matrix claim and names the skipped engines' {
        $narrowed = $manifestObject.PSObject.Copy()
        $narrowed.engines_skipped = @('4.5.1-stable')
        $body = Format-ValidationComment -Manifest $narrowed
        Assert-True (-not ($body -like '*on every Godot version*')) 'A narrowed run must not claim full-matrix coverage.'
        Assert-True ($body -like '*Unselected Godot versions*') 'Unselected engines must be named as scope notes.'
        Assert-True ($body -like '*4.5.1-stable*') 'The skipped engine was not named.'
        Assert-True ($body -like '*-GodotVersion 4.7.1-stable,4.6.1-stable*') 'The reproduce command should pin the narrowed scope.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: engine matrix selection'
    # --------------------------------------------------------------------------

    $supportedMatrix = @('4.7.1-stable', '4.6.1-stable', '4.5.1-stable')

    Test-Case 'no request runs the full matrix and is not marked narrowed' {
        $m = Select-EngineMatrix -Supported $supportedMatrix
        Assert-Equal 3 @($m.Selected).Count 'Expected the full matrix.'
        Assert-Equal 0 @($m.Skipped).Count 'Nothing should be skipped.'
        Assert-True (-not $m.Narrowed) 'A full matrix must not be flagged as narrowed.'
    }

    Test-Case 'a single requested version narrows the matrix and records the rest as skipped' {
        $m = Select-EngineMatrix -Supported $supportedMatrix -Requested @('4.6.1-stable')
        Assert-Equal 1 @($m.Selected).Count 'Expected exactly one engine.'
        Assert-Equal '4.6.1-stable' @($m.Selected)[0] 'Selected the wrong engine.'
        Assert-Equal 2 @($m.Skipped).Count 'Both other engines should be recorded as skipped.'
        Assert-True $m.Narrowed 'A narrowed matrix must be flagged.'
    }

    Test-Case 'selection follows manifest order, not request order, and ignores case' {
        $m = Select-EngineMatrix -Supported $supportedMatrix -Requested @('4.5.1-STABLE', '4.7.1-stable')
        Assert-Equal '4.7.1-stable' @($m.Selected)[0] 'Selection should preserve manifest order.'
        Assert-Equal '4.5.1-stable' @($m.Selected)[1] 'Selection should preserve manifest order.'
        Assert-Equal 1 @($m.Skipped).Count 'Expected one skipped engine.'
    }

    Test-Case 'requesting every supported version is not treated as narrowed' {
        $m = Select-EngineMatrix -Supported $supportedMatrix -Requested $supportedMatrix
        Assert-True (-not $m.Narrowed) 'Requesting the whole matrix is still full coverage.'
        Assert-Equal 0 @($m.Skipped).Count 'Nothing should be skipped.'
    }

    Test-Case 'an unsupported version is rejected rather than silently running nothing' {
        Assert-Throws -MatchPattern 'not in this pull request' -Body {
            Select-EngineMatrix -Supported $supportedMatrix -Requested @('4.4.0-stable')
        }
        Assert-Throws -MatchPattern '4\.7\.1-stable' -Body {
            Select-EngineMatrix -Supported $supportedMatrix -Requested @('4.4.0-stable')
        }
    }

    Test-Case 'an empty supported matrix is an error, not an empty run' {
        Assert-Throws -MatchPattern 'no supported Godot versions' -Body {
            Select-EngineMatrix -Supported @() -Requested @('4.6.1-stable')
        }
    }

    # --------------------------------------------------------------------------
    Write-Host 'PrLocalValidation: GDK selection and identity'
    # --------------------------------------------------------------------------

    $supportedGdk = @(
        [pscustomobject]@{ version = '2604.2.7849'; edition = '260402'; release = 'April 2026' },
        [pscustomobject]@{ version = '2604.1.7839'; edition = '260401'; release = 'April 2026' },
        [pscustomobject]@{ version = '2510.2.6247'; edition = '251002'; release = 'October 2025' }
    )

    Test-Case 'no request falls back to the candidate default and is not marked pinned' {
        $g = Select-GdkVersion -Supported $supportedGdk -Default '2604.1.7839'
        Assert-Equal '2604.1.7839' $g.Version 'Should build the candidate default.'
        Assert-Equal '260401' $g.Edition 'The declared edition should come from the manifest entry.'
        Assert-True (-not $g.Pinned) 'An unrequested GDK must not be reported as pinned.'

    }

    Test-Case 'a requested version is pinned and ignores case' {
        $g = Select-GdkVersion -Supported $supportedGdk -Default '2604.1.7839' -Requested '2604.2.7849'
        Assert-Equal '2604.2.7849' $g.Version 'Should build the requested version.'
        Assert-Equal '260402' $g.Edition 'Should carry the requested entry edition.'
        Assert-True $g.Pinned 'An explicit request must be reported as pinned.'

    }

    Test-Case 'an unsupported GDK is rejected and the error names the real choices' {
        Assert-Throws -MatchPattern 'not in this pull request' -Body {
            Select-GdkVersion -Supported $supportedGdk -Default '2604.1.7839' -Requested '2604.3.7874'
        }
        Assert-Throws -MatchPattern '2604\.2\.7849' -Body {
            Select-GdkVersion -Supported $supportedGdk -Default '2604.1.7839' -Requested '2604.3.7874'
        }
    }

    Test-Case 'a missing default or empty support list is an error, not an arbitrary SDK' {
        Assert-Throws -MatchPattern 'no default GDK version' -Body {
            Select-GdkVersion -Supported $supportedGdk -Default ''
        }
        Assert-Throws -MatchPattern 'no supported GDK versions' -Body {
            Select-GdkVersion -Supported @() -Default '2604.1.7839'
        }
    }

    Test-Case 'a default missing from the support list still builds but carries no declared edition' {
        $g = Select-GdkVersion -Supported $supportedGdk -Default '2604.9.9999'
        Assert-Equal '2604.9.9999' $g.Version 'The declared default should still be the expectation.'
        Assert-True ($null -eq $g.Edition) 'An unlisted default has no manifest edition to assert against.'
    }

    Test-Case 'the vcpkg override matches the shape CI injects' {
        $o = New-MsGdkOverride -Version '2604.2.7849'
        Assert-Equal 'ms-gdk' $o.name 'The override must target the ms-gdk port.'
        Assert-Equal '2604.2.7849' $o.version 'The override must carry the requested version.'
        $json = $o | ConvertTo-Json -Compress
        Assert-True ($json -like '*"name":"ms-gdk"*') "Unexpected override JSON: $json"
        Assert-Throws -MatchPattern 'non-version string' -Body { New-MsGdkOverride -Version 'latest' }
    }

    Test-Case 'the comment distinguishes a pinned GDK from an inherited one' {
        $inherited = Format-ValidationComment -Manifest $manifestObject
        Assert-True ($inherited -like '*inherited from the candidate checkout*') 'An unpinned run should say the SDK was inherited.'
        Assert-True (-not ($inherited -like '*-GdkVersion*')) 'An unpinned run must not suggest it pinned anything.'

        $pinned = $manifestObject.PSObject.Copy()
        $pinned | Add-Member -NotePropertyName 'gdk_pinned' -NotePropertyValue $true
        $pinned | Add-Member -NotePropertyName 'gdk_requested' -NotePropertyValue '2604.2.7849'
        $body = Format-ValidationComment -Manifest $pinned
        Assert-True ($body -like '*pinned by request*') 'A pinned run should say so.'
        Assert-True ($body -like '*-GdkVersion 2604.2.7849*') 'The reproduce command should carry the pinned SDK.'
    }

    Test-Case 'run directory names are commit-scoped and reject non-SHA input' {
        $name = New-RunDirectoryName -PullRequest 202 -HeadSha ('a' * 40) -TimestampUtc ([datetime]::new(2026, 1, 2, 3, 4, 5, [DateTimeKind]::Utc))
        Assert-Equal 'pr-202-aaaaaaaaaaaa-20260102T030405Z' $name 'Unexpected run directory name.'
        Assert-Throws -MatchPattern 'non-SHA' -Body { New-RunDirectoryName -PullRequest 202 -HeadSha 'main' -TimestampUtc ([datetime]::UtcNow) }
        Assert-Throws -MatchPattern 'must be positive' -Body { New-RunDirectoryName -PullRequest 0 -HeadSha ('a' * 40) -TimestampUtc ([datetime]::UtcNow) }
    }

    Test-Case 'only full 40-character SHAs are treated as commits' {
        Assert-True (Test-GitSha ('0123456789abcdef' + '0' * 24)) 'Valid SHA rejected.'
        Assert-True (-not (Test-GitSha 'ABCDEF0123456789ABCDEF0123456789ABCDEF01')) 'Uppercase SHA accepted; git reports lowercase.'
        Assert-True (-not (Test-GitSha 'abc1234')) 'Short SHA accepted.'
        Assert-True (-not (Test-GitSha '')) 'Empty SHA accepted.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'Engine acquisition call sites'
    # --------------------------------------------------------------------------

    # Install-Godot reuses an existing engine directory unless -Force is given.
    # Both callers extract into a location that can outlive a single run (a
    # self-hosted runner's runner.temp, or a reused -WorkRoot), so omitting
    # -Force would silently skip download + SHA-512 verification. The cache key
    # in setup-godot is hashed precisely to force that re-verification.
    Test-Case 'Install-Godot reuses an existing engine directory unless -Force is given' {
        $help = (Get-Command Install-Godot).Parameters
        Assert-True ($help.ContainsKey('Force')) 'Install-Godot must expose -Force for callers to demand re-verification.'
    }

    Test-Case 'get_godot.ps1 forwards -Force to Install-Godot' {
        $script = Get-Content -LiteralPath (Join-Path $ciRoot 'get_godot.ps1') -Raw
        Assert-True ($script.Contains('-Force:$Force')) 'get_godot.ps1 must forward its -Force switch.'
    }

    Test-Case 'the setup-godot cache-miss step re-verifies with -Force' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $actionPath = Join-Path $repoRoot '.github/actions/setup-godot/action.yml'
        $action = Get-Content -LiteralPath $actionPath -Raw
        Assert-True ($action.Contains('-Mode Install')) 'The action should still install through get_godot.ps1.'
        Assert-True ($action.Contains('-Force')) 'A cache miss must force a fresh, verified download.'
    }

    Test-Case 'the local wrapper also acquires engines with -Force' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $wrapper = Get-Content -LiteralPath (Join-Path $repoRoot 'tools/validate_pr_local.ps1') -Raw
        Assert-True ($wrapper.Contains('-ArchiveCacheDir $cacheDir -Force')) 'The wrapper must acquire engines with -Force.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'Credential scrubbing'
    # --------------------------------------------------------------------------

    # The clone is workspace isolation, not a sandbox, but the wrapper must at
    # least not hand its own live secrets to the code it is measuring.
    Test-Case 'candidate-controlled child processes run with credentials scrubbed' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $wrapper = Get-Content -LiteralPath (Join-Path $repoRoot 'tools/validate_pr_local.ps1') -Raw

        foreach ($name in @('PLAYFAB_DEVELOPER_SECRET_KEY', 'GITHUB_TOKEN', 'GH_TOKEN', 'LIVE_TESTS', 'LIVE_WRITE_TESTS')) {
            Assert-True ($wrapper.Contains("'$name'")) "$name must be on the scrub list."
        }

        $candidateCalls = @(
            "ArgumentList @('--preset', 'default') -LogPath `$buildLog -WorkingDirectory `$checkoutDir -ScrubCredentials",
            "ArgumentList @('--build', '--preset', 'debug') -LogPath `$buildLog -WorkingDirectory `$checkoutDir -ScrubCredentials",
            "ArgumentList `$legArgs -LogPath `$legLog -WorkingDirectory `$checkoutDir -ScrubCredentials",
            "ArgumentList @('--preset', 'default-release') -LogPath `$releaseLog -WorkingDirectory `$checkoutDir -ScrubCredentials",
            "ArgumentList @('--build', '--preset', 'release') -LogPath `$releaseLog -WorkingDirectory `$checkoutDir -ScrubCredentials"
        )
        foreach ($call in $candidateCalls) {
            Assert-True ($wrapper.Contains($call)) "A candidate-controlled call site is missing -ScrubCredentials: $call"
        }
    }

    # gh needs the token back, so scrubbing has to be scoped to the child call
    # rather than applied once for the whole run.
    Test-Case 'the publication path keeps its GitHub credentials' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $wrapper = Get-Content -LiteralPath (Join-Path $repoRoot 'tools/validate_pr_local.ps1') -Raw
        Assert-True ($wrapper.Contains("Invoke-Logged -FilePath 'gh'")) 'Could not locate the gh publication call.'
        Assert-True (-not ($wrapper -match "FilePath 'gh'[\s\S]{0,400}?-ScrubCredentials")) 'The trusted gh call must keep its credentials.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'Run-level verdict'
    # --------------------------------------------------------------------------

    $greenRun = @{
        LegStatuses       = @('pass', 'pass')
        ReleaseStatus     = 'pass'
    }

    Test-Case 'an all-green run passes' {
        Assert-Equal 'pass' (Get-RunVerdict @greenRun) 'A fully green run must pass.'
    }

    Test-Case 'a failed Release build fails the run' {
        $args = $greenRun.Clone()
        $args.ReleaseStatus = 'fail'
        Assert-Equal 'fail' (Get-RunVerdict @args) 'A failed Release build must fail the run.'
    }

    Test-Case 'one selected engine can pass independently' {
        Assert-Equal 'pass' (Get-RunVerdict -LegStatuses @('pass') -ReleaseStatus 'pass') 'Selected scope should stand on its own.'
        Assert-Equal 0 (Get-ExitCodeForStatus 'pass') 'Selected success must exit zero.'
    }

    Test-Case 'execution errors retain a non-passing verdict' {
        Assert-Equal 'error' (Get-RunVerdict @greenRun -ExecutionFailed $true) 'A later error must not hide behind completed legs.'
        Assert-Equal 'fail' (Get-RunVerdict -LegStatuses @('pass', 'fail') -ReleaseStatus 'pass') 'A failed leg still fails.'
    }
    # Nothing measured is not success.
    Test-Case 'a run with no engine legs is an error' {
        $args = $greenRun.Clone()
        $args.LegStatuses = @()
        Assert-Equal 'error' (Get-RunVerdict @args) 'A run that measured nothing must not pass.'
    }

    Test-Case 'incomplete maps to a nonzero exit code' {
        Assert-True ((Get-ExitCodeForStatus 'incomplete') -ne 0) 'An incomplete run must not exit 0.'
    }

    # --------------------------------------------------------------------------
    Write-Host 'Wrapper verdict and preflight call sites'
    # --------------------------------------------------------------------------

    Test-Case 'the wrapper grades the run through Get-RunVerdict' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $wrapper = Get-Content -LiteralPath (Join-Path $repoRoot 'tools/validate_pr_local.ps1') -Raw
        Assert-True ($wrapper -match '\$runStatus\s*=\s*Get-RunVerdict') 'The wrapper must fold its verdict through Get-RunVerdict.'
        foreach ($flag in @('-ExecutionFailed', '-ReleaseStatus', '-SandboxRestoreFailed', '-DebugStatus')) {
            Assert-True ($wrapper.Contains($flag)) "The wrapper must pass $flag to Get-RunVerdict."
        }
    }

    # The edition check used to live inside `if ($gdk.Pinned)`, so an unpinned
    # run that restored exactly the declared default still skipped it: a PR that
    # mapped that version to the wrong edition produced no identity gap and
    # passed. Gate it on "the restored package is the selected one" instead.
    Test-Case 'the declared edition is validated on the unpinned path too' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $wrapper = Get-Content -LiteralPath (Join-Path $repoRoot 'tools/validate_pr_local.ps1') -Raw

        Assert-True ($wrapper.Contains('if ($gdk.Edition -and $sdk.Package -ieq $gdk.Version -and $sdk.Edition -ine $gdk.Edition)')) 'The edition check must trigger whenever the restored package is the selected one.'
        # Package substitution stays pin-specific: an unpinned run resolving a
        # different version is the ordinary baseline case, reported as a gap.
        Assert-True ($wrapper.Contains('if ($gdk.Pinned -and $sdk.Package -ine $gdk.Version)')) 'The package-substitution check must stay pin-specific.'
        Assert-True (-not ($wrapper -match 'if \(\$gdk\.Pinned\) \{[\s\S]{0,600}?\$sdk\.Edition -ine')) 'The edition check must not be nested inside the pinned-only branch.'
    }


    # The machine-local mutex this wrapper used to take was removed on purpose.
    # This is a manually run tool, and a named mutex could not serialize against
    # the nightly workflow or another machine anyway, so it bought partial
    # protection at the cost of a lock lifetime to get wrong. Overlap is now an
    # operator responsibility, stated out loud at the start of every run rather
    # than half-enforced.
    Test-Case 'unserialized live writes are warned about rather than locked' {
        $repoRoot = Split-Path -Parent (Split-Path -Parent $ciRoot)
        $wrapper = Get-Content -LiteralPath (Join-Path $repoRoot 'tools/validate_pr_local.ps1') -Raw
        Assert-True (-not ($wrapper -match 'Mutex')) 'The wrapper must not reintroduce a machine-local lock.'
        Assert-True ($wrapper -match 'Nothing serializes those writes') 'The wrapper must say that nothing prevents a concurrent run.'
        Assert-True ($wrapper -match "Start one run at a time") 'The wrapper must tell the operator to run one at a time.'
    }

    # ----------------------------------------------------------------------
    Write-Host 'Uninterpretable summaries never escape the verdict'
    # ----------------------------------------------------------------------
    # The verdict is computed after the live writes have already happened, so a
    # throw here would skip the manifest, the report, and the PR comment for a
    # run that has already touched the shared title. Every one of these must
    # come back as an 'error' leg instead.

    Test-Case 'a non-numeric stage counter produces an error leg, not an exception' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Tests 'unknown'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A counter that cannot be read must not be graded as pass.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet "could not be interpreted") 'The reason must say the summary could not be interpreted.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet "gut:tests/godot/gdk") 'The reason must name the offending stage.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet "'tests'") 'The reason must name the offending field.'
    }

    Test-Case 'a non-numeric orchestrator count produces an error leg' {
        $mp = [pscustomobject]@{ summary = [pscustomobject]@{ total = 69; passed = 'n/a'; failed = 0; skipped = 0 } }
        $v = Get-EngineLegVerdict -Summary (New-Summary) -OrchestratorResult $mp -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'An unreadable scenario tally must not be graded as pass.'
    }

    Test-Case 'a non-numeric total_duration_ms produces an error leg' {
        $summary = New-Summary
        $summary.total_duration_ms = 'a while'
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A summary whose own fields cannot be read must not be graded as pass.'
    }

    Test-Case "a 'stages' value that is not a list of records produces an error leg" {
        $summary = [pscustomobject]@{
            overall_status = 'pass'; live = $true; live_writes = $true
            total_duration_ms = 1000; godot_version = '4.7.1-stable'
            stages = 'none'
        }
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A stages field of the wrong shape must not be graded as pass.'
    }

    Test-Case 'an error leg still carries every field the report and manifest read' {
        $summary = New-Summary
        $summary.total_duration_ms = 'a while'
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        foreach ($field in @('Status', 'Reasons', 'Gaps', 'Tests', 'Passing', 'Failing', 'Pending',
                             'MpTotal', 'MpPassed', 'MpFailed', 'MpSkipped', 'DurationMs', 'GodotVersion')) {
            Assert-True ([bool]$v.PSObject.Properties[$field]) "An error leg must still expose '$field'."
        }
        Assert-Equal 0 $v.Tests 'An uninterpretable leg must not claim test counts.'
        Assert-Equal 'unknown' $v.GodotVersion 'An uninterpretable leg must not claim an engine version.'
        Assert-Equal 0 @($v.Gaps).Count 'An uninterpretable leg reports a reason, not a coverage gap.'
    }

    Test-Case 'an error leg from a bad counter is rendered and exits nonzero' {
        $summary = New-Summary
        $summary.total_duration_ms = 'a while'
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        $run = Get-RunVerdict -LegStatuses @($v.Status) -ReleaseStatus 'pass'
        Assert-Equal 'error' $run 'An uninterpretable leg must fail the run.'
        Assert-True ((Get-ExitCodeForStatus $run) -ne 0) 'An uninterpretable leg must not exit 0.'
    }

    Test-Case 'a null or absent counter is still a legitimate null, not an error' {
        $stage = [pscustomobject]@{ name = 'gut:tests/godot/gdk'; status = 'pass'; passing = $null }
        Assert-True ($null -eq (Get-StageNumber -Stage $stage -Field 'passing')) 'An explicit null counter reads as null.'
        Assert-True ($null -eq (Get-StageNumber -Stage $stage -Field 'tests')) 'An absent counter reads as null.'
        Assert-Equal 7 (Get-StageNumber -Stage ([pscustomobject]@{ tests = 7 }) -Field 'tests') 'A real counter still reads as a number.'
    }

    Test-Case 'Get-StageNumber names the stage and field it could not read' {
        $stage = [pscustomobject]@{ name = 'gut:tests/godot/playfab'; tests = 'unknown' }
        Assert-Throws -MatchPattern "gut:tests/godot/playfab.*'tests'.*unknown" -Body {
            Get-StageNumber -Stage $stage -Field 'tests'
        }
    }

    # ----------------------------------------------------------------------
    Write-Host 'Live flags, host tallies and the parse gate are graded strictly'
    # ----------------------------------------------------------------------
    # Each of these is a way a summary can look green while proving nothing:
    # a stringified live flag, a host that discovered tests but reported no
    # passes, and a parse gate that ran and failed.

    Test-Case 'a stringified live flag is not coerced into a live claim' {
        $summary = New-Summary
        $summary.live = 'false'
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A non-boolean live flag must not grade as a live run.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet "'live'") 'The reason must name the live flag.'
    }

    Test-Case 'a stringified live_writes flag is not coerced into a live-write claim' {
        $summary = New-Summary
        $summary.live_writes = 'no'
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A non-boolean live_writes flag must not grade as a live-write run.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet "'live_writes'") 'The reason must name the live_writes flag.'
    }

    Test-Case 'a null live flag is an error, not a live run' {
        $summary = New-Summary
        $summary.live = $null
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A null live flag must not grade as a live run.'
    }

    Test-Case 'real boolean live flags are still read normally' {
        Assert-True (Get-SummaryFlag -Summary (New-Summary) -Field 'live') 'A JSON true must read as true.'
        Assert-True (-not (Get-SummaryFlag -Summary (New-Summary -Live $false) -Field 'live')) 'A JSON false must read as false.'
        $v = Get-EngineLegVerdict -Summary (New-Summary -LiveWrites $false) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'error' $v.Status 'A genuinely non-live-write run is still an error leg.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet 'AllowLiveWrites') 'The existing live-write reason must survive.'
    }

    Test-Case 'a GUT host that discovered tests but reported no passing count is incomplete' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Tests 12 -Passing $null),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'incomplete' $v.Status 'A host with no passing count must not grade as pass.'
        Assert-True (@($v.Gaps) -join ' ' | Select-String -Quiet 'gut:tests/godot/gdk') 'The gap must name the host.'
        Assert-True (@($v.Gaps) -join ' ' | Select-String -Quiet 'no passing count') 'The gap must say the passing count was missing.'
    }

    Test-Case 'a host that passed zero tests still reports the all-skipped gap' {
        $stages = @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Tests 12 -Passing 0 -Pending 12),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'incomplete' $v.Status 'A host that passed nothing must not grade as pass.'
        Assert-True (@($v.Gaps) -join ' ' | Select-String -Quiet 'passed no tests') 'The all-skipped gap must still be reported.'
    }

    Test-Case 'a failed parse gate fails the leg even when overall_status disagrees' {
        $stages = @(
            (New-Stage -Name 'parse-gate' -Status 'fail'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'fail' $v.Status 'A failed parse gate must fail the leg.'
        Assert-True (@($v.Reasons) -join ' ' | Select-String -Quiet 'parse-gate') 'The reason must name the parse gate.'
    }

    Test-Case 'a skipped parse gate is still a coverage gap, not a failure' {
        $stages = @(
            (New-Stage -Name 'parse-gate' -Status 'skip'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk'),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest
        Assert-Equal 'incomplete' $v.Status 'A skipped parse gate stays a gap.'
        Assert-True (@($v.Gaps) -join ' ' | Select-String -Quiet 'parse-gate did not run') 'The skip gap must survive.'
    }

    # ----------------------------------------------------------------------
    Write-Host ''
    Write-Host 'Coverage, matrix and verdict inputs come from what actually ran'
    # ----------------------------------------------------------------------

    Test-Case 'a comma-delimited Godot request is accepted' {
        # pwsh -File binds '-GodotVersion a,b' as one string, and the report's
        # own reproduction command prints exactly that form.
        $m = Select-EngineMatrix -Supported @('4.7.1-stable', '4.6.1-stable', '4.5.1-stable') -Requested @('4.7.1-stable,4.6.1-stable')
        Assert-Equal 2 @($m.Selected).Count 'Both requested versions must be selected.'
        Assert-Equal '4.7.1-stable' $m.Selected[0] 'Manifest order is preserved.'
        Assert-Equal '4.6.1-stable' $m.Selected[1] 'Manifest order is preserved.'
        Assert-True $m.Narrowed 'A two-of-three request is still a narrowed run.'
    }

    Test-Case 'a comma-delimited request tolerates spaces and still rejects unknown versions' {
        $m = Select-EngineMatrix -Supported @('4.7.1-stable', '4.6.1-stable') -Requested @(' 4.6.1-stable , ')
        Assert-Equal 1 @($m.Selected).Count 'Padding and trailing separators must not invent versions.'
        Assert-Equal '4.6.1-stable' $m.Selected[0] 'The named version is selected.'
        Assert-Throws -Body { Select-EngineMatrix -Supported @('4.7.1-stable') -Requested @('4.7.1-stable,9.9.9-stable') } -Match 'not in this pull request'
    }

    Test-Case 'the wrapper protects and records sandbox restoration' {
        $src = Get-Content -LiteralPath (Join-Path (Split-Path -Parent (Split-Path -Parent $ciRoot)) 'tools/validate_pr_local.ps1') -Raw
        $tryIndex = $src.IndexOf('$legs = @()')
        $switchIndex = $src.IndexOf('Set-CurrentSandbox -ExePath $sandboxExe -SandboxId $script:SandboxId')
        $restoreIndex = $src.IndexOf('Restoring Xbox sandbox to')
        Assert-True ($tryIndex -gt 0 -and $switchIndex -gt 0 -and $restoreIndex -gt 0) 'The switch and its restore must both exist.'
        Assert-True ($switchIndex -gt $tryIndex -and $switchIndex -lt $restoreIndex) 'A switch that half-succeeds must still reach the restore.'
        Assert-True ($src -match '\$sandboxRestoreError = \$_\.Exception\.Message') 'A failed sandbox restore must be recorded, not thrown.'
        # The flag, not the message, must drive the verdict and the manifest:
        # an exception can carry an empty Message, and [bool]'' is $false,
        # which would publish a PASS claiming the sandbox was restored.
        Assert-True ($src -match '\$sandboxRestoreFailed = \$true') 'A failed restore must set an explicit flag.'
        Assert-True ($src -match '-SandboxRestoreFailed \$sandboxRestoreFailed') 'The verdict must read the flag, not the message.'
        Assert-True ($src -match 'sandbox_restored\s+= if \(\$sandboxAttempted\) \{ -not \$sandboxRestoreFailed \} else \{ \$null \}') 'The manifest must distinguish failed, confirmed and unattempted restores.'
        Assert-True ($src -notmatch '\[bool\]\$sandboxRestoreError') 'An empty exception message must not read as a successful restore.'
        # Write-Warning is itself terminating under -WarningAction Stop, so a
        # warning that reports a degraded path must opt out of preference
        # escalation or it reintroduces the throw it was written to replace.
        Assert-True ($src -match 'Write-Warning -WarningAction Continue "FAILED to restore the Xbox sandbox') 'The restore warning must not be escalatable.'
        $restoreErrIndex = $src.IndexOf('$sandboxRestoreError = $_.Exception.Message')
        # Matched loosely: pinning the exact padding would fail on a column
        # realignment that changes nothing about the behaviour under test.
        $manifestIndex = [regex]::Match($src, 'schema_version\s+= 1').Index
        Assert-True ($restoreErrIndex -gt 0 -and $manifestIndex -gt $restoreErrIndex) 'Evidence must still be written after a failed restore.'
    }

    # ----------------------------------------------------------------------
    Write-Host ''
    Write-Host 'Signed-in Xbox evidence is graded when the candidate reports it'
    # ----------------------------------------------------------------------
    # A host-wide passing tally cannot distinguish a signed-in run from a
    # signed-out one, because the Xbox-backed suites mark a missing identity
    # pending and still pass their contract tests. The summary field is optional
    # so that branches predating it are not permanently incomplete, but a
    # candidate that answers is held to its answer.
    Test-Case 'a leg that reports a signed-out machine cannot settle as a pass' {
        $summary = New-Summary
        $summary | Add-Member -NotePropertyName 'xbox_signed_in' -NotePropertyValue $false
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'incomplete' $v.Status 'A signed-out leg must not pass.'
        Assert-Equal 'signed-out' $v.XboxIdentity 'The identity state must be recorded.'
        Assert-True (@($v.Gaps | Where-Object { $_ -match 'signed-in Xbox identity' }).Count -gt 0) 'The gap must name the missing identity.'
    }

    Test-Case 'a leg that reports a signed-in machine passes and records the evidence' {
        $summary = New-Summary
        $summary | Add-Member -NotePropertyName 'xbox_signed_in' -NotePropertyValue $true
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'pass' $v.Status "Reported sign-in should not change the verdict: $($v.Reasons -join '; ') / $($v.Gaps -join '; ')"
        Assert-Equal 'signed-in' $v.XboxIdentity 'Confirmed sign-in must be recorded as such.'
    }

    Test-Case 'a candidate that cannot report sign-in is not failed for it' {
        $v = Get-EngineLegVerdict -Summary (New-Summary) -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'pass' $v.Status 'An older candidate must still be gradeable.'
        Assert-Equal 'unreported' $v.XboxIdentity 'Absent evidence is unreported, not confirmed.'
    }

    Test-Case 'a non-boolean sign-in field is an uninterpretable summary' {
        # Same trap as live/live_writes: [bool]'false' is $true, so a drifted
        # schema would manufacture the exact evidence being demanded.
        $summary = New-Summary
        $summary | Add-Member -NotePropertyName 'xbox_signed_in' -NotePropertyValue 'false'
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'error' $v.Status 'A non-boolean flag must not be coerced.'
        Assert-Equal 'unreported' $v.XboxIdentity 'An unreadable summary proves no identity.'
    }

    Test-Case 'the report names the legs whose sign-in state is unknown' {
        $m = $manifestObject.PSObject.Copy()
        $body = Format-ValidationComment -Manifest $m
        Assert-True ($body -like '*Signed-in Xbox identity on*') 'An unreported leg must be disclosed as not covered.'
        Assert-True ($body -like '*4.7.1-stable*') 'The unreported leg must be named.'
    }

    Test-Case 'the report drops the sign-in caveat once every leg confirms it' {
        $m = $manifestObject.PSObject.Copy()
        $m.engines = @(
            [pscustomobject]@{ version = '4.7.1-stable'; status = 'pass'; tests = 30; passing = 30; failing = 0; pending = 0; duration_ms = 61000; reasons = @(); gaps = @(); xbox_identity = 'signed-in' },
            [pscustomobject]@{ version = '4.6.1-stable'; status = 'incomplete'; tests = 20; passing = 20; failing = 0; pending = 0; duration_ms = 55000; reasons = @(); gaps = @(); xbox_identity = 'signed-out' }
        )
        $body = Format-ValidationComment -Manifest $m
        Assert-True (-not ($body -like '*Signed-in Xbox identity on*')) 'A reported leg is graded, not listed as unknown.'
    }

    # ----------------------------------------------------------------------
    Write-Host ''
    Write-Host 'Reported failures and an unrestored sandbox are graded independently'
    # ----------------------------------------------------------------------
    # A stage's status field and the tallies beside it come from the same file,
    # written by the run being graded. Where the two disagree, the count is the
    # harder evidence, and the report would otherwise print a PASS heading over
    # a nonzero Failed column.
    Test-Case 'a host reporting failures fails the leg even when its status says pass' {
        $summary = New-Summary -Stages @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Status 'pass' -Tests 10 -Passing 9 -Failing 1),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'fail' $v.Status 'A reported failure must fail the leg on its own.'
        Assert-True (@($v.Reasons | Where-Object { $_ -match "gut:tests/godot/gdk reported 1 failing test\(s\) despite status='pass'" }).Count -gt 0) 'The reason must name the host and the disagreement.'
        Assert-Equal 1 $v.Failing 'The failing count must still be reported.'
    }

    Test-Case 'a host reporting failures with no discovered tests is still a failure' {
        # The tally is read before the test-count branch, so a host that also
        # reports nothing discovered cannot swallow its own failures.
        $summary = New-Summary -Stages @(
            (New-Stage -Name 'parse-gate'),
            (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Status 'pass' -Tests 0 -Passing 0 -Failing 2),
            (New-Stage -Name 'gut:tests/godot/playfab'),
            (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'playfab-multiplayer-orchestrator' -Tests 4 -Passing 4)
        )
        $v = Get-EngineLegVerdict -Summary $summary -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'fail' $v.Status 'A failure outranks the no-tests coverage gap.'
        Assert-Equal 2 $v.Failing 'The failing count must be carried into the report.'
    }

    Test-Case 'a clean host tally is unaffected by the failure check' {
        $v = Get-EngineLegVerdict -Summary (New-Summary) -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'pass' $v.Status "A zero failing count must not fail the leg: $($v.Reasons -join '; ')"
        Assert-Equal 0 $v.Failing 'No failures were reported.'
    }

    # A host that ran tests and then declined to say how many failed has not
    # shown that none did. The same rule already applies to the passing count:
    # an absent tally is as unconfirmed as a zero one, and $null coerces to 0
    # on the way into the report's Failed column.
    Test-Case 'a host that reports no failing count cannot confirm a clean result' {
        $stages = @(
            (New-Stage -Name 'parse-gate'), (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Failing $null),
            (New-Stage -Name 'gut:tests/godot/playfab'), (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'bootstrap:tests/godot/gdk'), (New-Stage -Name 'bootstrap:tests/godot/playfab'),
            (New-Stage -Name 'bootstrap:tests/godot/gameinput')
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'incomplete' $v.Status 'An absent failure tally cannot support a pass.'
        Assert-True (@($v.Gaps | Where-Object { $_ -match 'no failing count' }).Count -gt 0) 'The gap must name the missing tally.'
    }

    Test-Case 'a negative failing count is an unreadable result, not a pass' {
        # -1 is not greater than zero, so a bare "> 0" guard would wave it
        # through as a clean host.
        $stages = @(
            (New-Stage -Name 'parse-gate'), (New-Stage -Name 'cpp-doctest'),
            (New-Stage -Name 'gut:tests/godot/gdk' -Failing -1),
            (New-Stage -Name 'gut:tests/godot/playfab'), (New-Stage -Name 'gut:tests/godot/gameinput'),
            (New-Stage -Name 'bootstrap:tests/godot/gdk'), (New-Stage -Name 'bootstrap:tests/godot/playfab'),
            (New-Stage -Name 'bootstrap:tests/godot/gameinput')
        )
        $v = Get-EngineLegVerdict -Summary (New-Summary -Stages $stages) -RequireOrchestrator -RequireDoctest -OrchestratorResult $null
        Assert-Equal 'error' $v.Status 'An impossible tally must not be graded as a result.'
        Assert-True (@($v.Reasons | Where-Object { $_ -match 'negative failing count' }).Count -gt 0) 'The reason must name the bad value.'
    }

    # A sandbox this run could not put back is an operator action item, not a
    # statement about the candidate's code -- but it must never sit under a PASS
    # heading, and the evidence must still be published.
    Test-Case 'a failed sandbox restore errors the run verdict' {
        $status = Get-RunVerdict -LegStatuses @('pass') -ReleaseStatus 'pass' -SandboxRestoreFailed $true
        Assert-Equal 'error' $status 'A machine left in the test sandbox cannot be a pass.'
    }

    Test-Case 'a restored sandbox leaves an otherwise clean run passing' {
        $status = Get-RunVerdict -LegStatuses @('pass') -ReleaseStatus 'pass' -SandboxRestoreFailed $false
        Assert-Equal 'pass' $status 'A successful restore must not degrade the verdict.'
    }

    Test-Case 'the report says plainly when the sandbox was not restored' {
        $m = $manifestObject.PSObject.Copy()
        $m | Add-Member -NotePropertyName 'sandbox_restored' -NotePropertyValue $false -Force
        $body = Format-ValidationComment -Manifest $m
        Assert-True ($body -like '*NOT restored*') 'An unrestored sandbox must be stated, not implied.'
        Assert-True (-not ($body -match '\(restored to `RETAIL` afterwards\)')) 'The success wording must not survive a failed restore.'
    }

    Test-Case 'the report keeps the restored wording for a successful restore' {
        $m = $manifestObject.PSObject.Copy()
        $m | Add-Member -NotePropertyName 'sandbox_restored' -NotePropertyValue $true -Force
        $body = Format-ValidationComment -Manifest $m
        Assert-True ($body -match 'restored to .RETAIL. afterwards') 'A confirmed restore reads as before.'
        Assert-True (-not ($body -like '*NOT restored*')) 'A confirmed restore must not warn.'
    }

    Test-Case 'a manifest predating the restore flag still renders' {
        # Set-StrictMode would throw on the absent property if it were read
        # directly, and an older manifest should read as it always did.
        $m = $manifestObject.PSObject.Copy()
        $body = Format-ValidationComment -Manifest $m
        Assert-True ($body -match 'restored to .RETAIL. afterwards') 'An absent flag keeps the original wording.'
    }

    # ----------------------------------------------------------------------
    Write-Host 'Debug failures use the real wrapper reporting path'
    # ----------------------------------------------------------------------
    $wrapperPath = Join-Path (Split-Path -Parent (Split-Path -Parent $ciRoot)) 'tools\validate_pr_local.ps1'
    $wrapperSource = Get-Content -LiteralPath $wrapperPath -Raw
    $flowStart = $wrapperSource.IndexOf("    `$debugStatus = 'error'")
    $flowEnd = $wrapperSource.IndexOf("`n} catch {", $flowStart)
    if ($flowStart -lt 0 -or $flowEnd -lt 0) { throw 'Could not locate the wrapper execution/reporting block.' }
    $failureFlow = [scriptblock]::Create($wrapperSource.Substring($flowStart, $flowEnd - $flowStart))

    function Invoke-DebugFailureFixture {
        param([string]$FailureMode, [bool]$WithoutComment = $false)
        $runDir = Join-Path $fixtureRoot ([guid]::NewGuid().ToString('n'))
        $logsDir = Join-Path $runDir 'logs'
        New-Item -ItemType Directory -Path $logsDir -Force | Out-Null
        $checkoutDir = Join-Path $runDir 'checkout'
        $enginesDir = Join-Path $runDir 'engines'
        $resultsDir = Join-Path $runDir 'results'
        $cacheDir = Join-Path $runDir 'cache'
        $manifestPath = Join-Path $runDir 'godot-versions.json'
        $sandboxExe = 'fixture-sandbox.exe'
        $PullRequest = 203
        $headSha = 'f' * 40
        $startedAt = [datetime]::UtcNow
        $originalSandbox = 'lykhvw.0'
        $pr = [pscustomobject]@{ url = 'https://github.com/microsoft/XBOX-Godot-Sample/pull/203' }
        $gdk = [pscustomobject]@{ Version = '2604.2.7849'; Edition = '260402'; Pinned = $true }
        $GdkVersion = '2604.2.7849'
        $versions = @('4.6.1-stable')
        $gdkManifest = [pscustomobject]@{ Default = '2604.2.7849' }
        $matrix = [pscustomobject]@{ Narrowed = $true; Selected = $versions; Skipped = @('4.7.1-stable') }
        if ($FailureMode -eq 'inherited') {
            $gdk.Pinned = $false
            $gdk.Version = '2604.9.9999'
            $gdk.Edition = $null
            $gdkManifest.Default = $gdk.Version
            $GdkVersion = $null
        }
        $NoComment = $WithoutComment
        $script:Repository = 'microsoft/XBOX-Godot-Sample'
        $script:PlayFabTitleId = '10D176'
        $script:PlayFabCustomId = 'godot-gdk-ext-live-smoke'
        $script:PlayFabMatchmaking = 'godot_gdk_ext_live_smoke_queue'
        $script:SandboxId = 'lykhvw.0'
        $calls = [System.Collections.Generic.List[string]]::new()
        $WarningPreference = 'Stop'

        function Write-Phase { param($Message) }
        function Write-Note { param($Message) }
        function Get-SdkIdentity {
            if ($FailureMode -eq 'sdk') { throw 'LOCAL-ONLY-DETAIL SDK identity is missing' }
            return [pscustomobject]@{ Edition = '260402'; Package = '2604.2.7849' }
        }
        function pwsh {
            param([switch]$NoLogo, [switch]$NoProfile, $File, $Mode, $Version, $ManifestPath, $DestinationRoot, $ArchiveCacheDir, [switch]$Force)
            if ($FailureMode -eq 'acquire') { throw 'LOCAL-ONLY-DETAIL engine acquisition failed' }
            Set-Variable -Name LASTEXITCODE -Scope 1 -Value 0
            return (@{ Path = (Join-Path $enginesDir "$Version\godot.exe") } | ConvertTo-Json -Compress)
        }
        function Copy-RedistributableDlls { param($CheckoutRoot, $EngineDir) return 1 }
        function Set-CurrentSandbox { param($ExePath, $SandboxId, $LogPath) return $SandboxId }
        function Get-CurrentSandbox {
            param($ExePath)
            if ($FailureMode -eq 'restore') { throw 'sandbox restore failed' }
            return $originalSandbox
        }
        function Invoke-Logged {
            param($FilePath, $ArgumentList, $LogPath, $WorkingDirectory, [switch]$ScrubCredentials)
            $command = "$FilePath $($ArgumentList -join ' ')"
            $calls.Add($command)
            if ($FilePath -eq 'gh') {
                $bodyIndex = [array]::IndexOf($ArgumentList, '--body-file')
                Assert-True ($bodyIndex -ge 0) 'Publication must use the saved report.'
                Assert-True (Test-Path -LiteralPath $ArgumentList[$bodyIndex + 1]) 'The report must exist before publication.'
                return 0
            }
            if ($FilePath -eq 'pwsh') {
                $outIndex = [array]::IndexOf($ArgumentList, '-OutDir')
                $out = $ArgumentList[$outIndex + 1]
                New-Item -ItemType Directory -Force -Path $out | Out-Null
                $summary = New-Summary
                $summary.godot_version = '4.6.1.stable.official.fixture'
                if ($FailureMode -eq 'wrong-engine') { $summary.godot_version = '4.7.1.stable.official.fixture' }
                if ($FailureMode -eq 'missing-engine') { $summary.PSObject.Properties.Remove('godot_version') }
                $summary | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $out 'run-summary.json')
                return 0
            }
            Assert-Equal 'cmake' $FilePath 'Only simulated build, test and publication commands may execute.'
            if ($FailureMode -eq 'release' -and $command -eq 'cmake --preset default-release') {
                throw 'LOCAL-ONLY-DETAIL Release could not start'
            }
            if ($FailureMode -eq 'launch') { throw 'LOCAL-ONLY-DETAIL C:\private\tool.exe could not start' }
            if ($FailureMode -eq 'configure' -or ($FailureMode -eq 'compile' -and $command -eq 'cmake --build --preset debug')) { return 17 }
            Add-Content -LiteralPath $LogPath -Value $command
            return 0
        }

        & $failureFlow | Out-Null
        return [pscustomobject]@{
            Manifest = Get-Content -LiteralPath (Join-Path $runDir 'validation-manifest.json') -Raw | ConvertFrom-Json
            Report = Get-Content -LiteralPath (Join-Path $runDir 'report.md') -Raw
            Calls = @($calls.ToArray())
            Log = if (Test-Path -LiteralPath (Join-Path $logsDir 'execution-errors.log')) {
                Get-Content -LiteralPath (Join-Path $logsDir 'execution-errors.log') -Raw
            } else { '' }
        }
    }

    foreach ($mode in @('configure', 'compile', 'launch')) {
        Test-Case "a Debug $mode failure writes and publishes honest failure evidence" {
            $result = Invoke-DebugFailureFixture -FailureMode $mode
            $expected = if ($mode -eq 'launch') { 'error' } else { 'fail' }
            Assert-Equal $expected $result.Manifest.status 'The overall verdict must reflect the Debug failure.'
            Assert-Equal $expected $result.Manifest.debug_build_status 'The Debug verdict must be recorded.'
            $step = if ($mode -eq 'compile') { 'cmake --build --preset debug' } else { 'cmake --preset default' }
            Assert-Equal $step $result.Manifest.debug_build_step 'The failing step must be identified.'
            Assert-Equal 'not run' $result.Manifest.release_build_status 'Release must not execute.'
            Assert-Equal 'not run' $result.Manifest.live_matrix_status 'Live tests must not execute.'
            Assert-Equal 0 @($result.Manifest.engines).Count 'No engine evidence may be invented.'
            Assert-True ($null -eq $result.Manifest.gdk_edition) 'The resolved edition must stay unknown.'
            Assert-True ($null -eq $result.Manifest.ms_gdk_version) 'The resolved package must stay unknown.'
            Assert-True (-not $result.Manifest.sandbox_attempted) 'The sandbox must not be touched.'
            Assert-True ($null -eq $result.Manifest.sandbox_restored) 'An unattempted restore must not be reported as confirmed.'
            Assert-True ($result.Report -match "Local PR validation - $($expected.ToUpperInvariant())") 'The heading must not pass.'
            Assert-True ($result.Report -match 'unavailable') 'Unknown SDK identity must be explicit.'
            Assert-True ($result.Report -match 'not changed') 'The report must not claim a sandbox switch/restore.'
            Assert-True ($result.Report -like '*-GodotVersion 4.6.1-stable -GdkVersion 2604.2.7849*') 'Reproduction must use requested inputs even before the build resolves an SDK.'
            Assert-True (-not ($result.Report -match 'Ran ``tools/run_all_tests|on every Godot|so it ran on the first')) 'The report must not claim tests ran.'
            Assert-True (-not ($result.Report -match 'LOCAL-ONLY-DETAIL|C:\\private')) 'Local exception details must not be published.'
            Assert-True ($result.Calls[-1] -match '^gh pr comment ') 'A failed build must still publish its report.'
            Assert-Equal $(if ($mode -eq 'compile') { 3 } else { 2 }) $result.Calls.Count 'Execution must stop at the failed Debug step, then publish.'
        }
    }

    Test-Case 'NoComment preserves Debug failure artifacts without publishing' {
        $result = Invoke-DebugFailureFixture -FailureMode 'configure' -WithoutComment $true
        Assert-Equal 1 $result.Calls.Count 'NoComment must suppress publication without losing artifacts.'
        Assert-Equal 'fail' $result.Manifest.status 'The saved failure remains a failure.'
    }

    foreach ($mode in @('success', 'inherited')) {
        Test-Case "selected scope passes with $mode SDK selection" {
            $result = Invoke-DebugFailureFixture -FailureMode $mode
            Assert-Equal 'pass' $result.Manifest.status "One requested engine should pass independently. $($result.Log)"
            Assert-Equal 1 @($result.Manifest.engines).Count 'Only the selected engine should run.'
            Assert-Equal '4.6.1-stable' $result.Manifest.engines[0].version 'The selected identity must be recorded.'
            Assert-Equal '2604.2.7849' $result.Manifest.ms_gdk_version 'The actual SDK, not the declared default, defines the scope.'
            Assert-Equal 'pass' $result.Manifest.release_build_status 'Release must complete.'
            Assert-True ($result.Report -like '*Unselected Godot versions: 4.7.1-stable*') 'Other engines remain visible as scope notes.'
            Assert-True (-not ($result.Report -match 'Coverage gaps|full-matrix evidence')) 'Selection alone must not introduce a coverage gap.'
        }
    }

    foreach ($mode in @('sdk', 'acquire', 'release', 'restore', 'wrong-engine', 'missing-engine')) {
        Test-Case "a $mode error retains and publishes available results" {
            $result = Invoke-DebugFailureFixture -FailureMode $mode
            Assert-Equal 'error' $result.Manifest.status 'An execution or identity error cannot pass.'
            Assert-Equal 'pass' $result.Manifest.debug_build_status 'Debug results must be retained.'
            $hasLeg = $mode -in @('release', 'restore', 'wrong-engine', 'missing-engine')
            Assert-Equal ([int]$hasLeg) @($result.Manifest.engines).Count "Completed leg records must survive later errors. $($result.Log)"
            Assert-True ($result.Calls[-1] -match '^gh pr comment ') 'Later errors must still publish.'
            Assert-True (-not ($result.Report -match 'LOCAL-ONLY-DETAIL')) 'Raw execution errors remain in local logs.'
            if ($mode -eq 'release') {
                Assert-Equal 'pass' $result.Manifest.engines[0].status 'Completed live evidence must survive a Release launch error.'
                Assert-Equal 'error' $result.Manifest.release_build_status 'An unlaunched Release build cannot read as pass.'
            }
            if (-not $hasLeg) {
                Assert-Equal 1 @($result.Manifest.engines_not_run).Count 'Uncompleted selections must be named.'
                Assert-Equal 'not run' $result.Manifest.release_build_status 'Unreached Release must not claim success.'
            }
        }
    }

} finally {
    Remove-Item -LiteralPath $fixtureRoot -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host ''
Write-Host "$script:Passed passed, $script:Failed failed." -ForegroundColor $(if ($script:Failed -eq 0) { 'Green' } else { 'Red' })
exit ([int]($script:Failed -gt 0))
