#Requires -Version 7.0
<#
.SYNOPSIS
    Offline contract tests for the PR-local validation tooling.

.DESCRIPTION
    Runs with no network, no Godot, no GDK, and no PlayFab. It covers the two
    modules that decide what a validation run is allowed to claim:

      tools\ci\GodotAcquisition.psm1  - supply-chain refusal rules
      tools\ci\PrLocalValidation.psm1 - verdict honesty and Markdown safety

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

    Test-Case 'a full-matrix comment claims full coverage and omits the narrowing notice' {
        $body = Format-ValidationComment -Manifest $manifestObject
        Assert-True ($body -like '*on every Godot version*') 'A full run should claim full-matrix coverage.'
        Assert-True (-not ($body -like '*narrowed by request*')) 'A full run must not claim it was narrowed.'
    }

    Test-Case 'a narrowed comment drops the full-matrix claim and names the skipped engines' {
        $narrowed = $manifestObject.PSObject.Copy()
        $narrowed | Add-Member -NotePropertyName 'engines_skipped' -NotePropertyValue @('4.5.1-stable')
        $body = Format-ValidationComment -Manifest $narrowed
        Assert-True (-not ($body -like '*on every Godot version*')) 'A narrowed run must not claim full-matrix coverage.'
        Assert-True ($body -like '*narrowed by request*') 'The narrowing notice is missing.'
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
        Assert-Equal 2 @($g.Uncovered).Count 'The other supported versions are uncovered.'
    }

    Test-Case 'a requested version is pinned and ignores case' {
        $g = Select-GdkVersion -Supported $supportedGdk -Default '2604.1.7839' -Requested '2604.2.7849'
        Assert-Equal '2604.2.7849' $g.Version 'Should build the requested version.'
        Assert-Equal '260402' $g.Edition 'Should carry the requested entry edition.'
        Assert-True $g.Pinned 'An explicit request must be reported as pinned.'
        Assert-True (@($g.Uncovered) -notcontains '2604.2.7849') 'The built version must not be listed as uncovered.'
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

    Test-Case 'uncovered editions are only reported when the pull request edits the support list' {
        $none = Get-GdkCoverageGap -Selected '2604.1.7839' -Uncovered @('2510.2.6247') -ManifestChanged $false -PullRequest 202
        Assert-True ($null -eq $none) 'Every run leaves editions unbuilt; reporting that always would be noise.'

        $gap = Get-GdkCoverageGap -Selected '2604.1.7839' -Uncovered @('2510.2.6247') -ManifestChanged $true -PullRequest 202
        Assert-True ($gap -like '*2510.2.6247*') 'The unvalidated edition was not named.'
        Assert-True ($gap -like '*-GdkVersion*') 'The gap must say how to cover the remaining editions.'
    }

    Test-Case 'a support-list change with nothing left uncovered reports no gap' {
        $gap = Get-GdkCoverageGap -Selected '2604.2.7849' -Uncovered @() -ManifestChanged $true -PullRequest 202
        Assert-True ($null -eq $gap) 'Full coverage must not produce a gap.'
    }

    Test-Case 'building an SDK other than the declared default is reported as a gap' {
        $gap = Get-GdkIdentityGap -Expected '2604.2.7849' -Restored '2604.1.7839' -Pinned $false
        Assert-True ($gap -like '*2604.2.7849*') 'The expected version is missing from the gap.'
        Assert-True ($gap -like '*2604.1.7839*') 'The actually-built version is missing from the gap.'
        Assert-True ($gap -like '*registry baseline*') 'The gap must explain why the two can differ.'
    }

    Test-Case 'a matching or deliberately pinned SDK produces no identity gap' {
        Assert-True ($null -eq (Get-GdkIdentityGap -Expected '2604.2.7849' -Restored '2604.2.7849' -Pinned $false)) 'A matching SDK is not a gap.'
        Assert-True ($null -eq (Get-GdkIdentityGap -Expected '2604.1.7839' -Restored '2604.2.7849' -Pinned $true)) 'A pinned run deliberately overrides the default.'
    }

    Test-Case 'the comment distinguishes a pinned GDK from an inherited one' {
        $inherited = Format-ValidationComment -Manifest $manifestObject
        Assert-True ($inherited -like '*inherited from the candidate checkout*') 'An unpinned run should say the SDK was inherited.'
        Assert-True (-not ($inherited -like '*-GdkVersion*')) 'An unpinned run must not suggest it pinned anything.'

        $pinned = $manifestObject.PSObject.Copy()
        $pinned | Add-Member -NotePropertyName 'gdk_pinned' -NotePropertyValue $true
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
} finally {
    Remove-Item -LiteralPath $fixtureRoot -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host ''
Write-Host "$script:Passed passed, $script:Failed failed." -ForegroundColor $(if ($script:Failed -eq 0) { 'Green' } else { 'Red' })
exit ([int]($script:Failed -gt 0))
