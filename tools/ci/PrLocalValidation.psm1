<#
.SYNOPSIS
    Pure helpers for tools\validate_pr_local.ps1 (PR-local live-write validation).

.DESCRIPTION
    Everything in this module is deterministic and side-effect free so it can be
    covered offline by tools\ci\tests\pr_local_validation.test.ps1. The process,
    network, Git, and Xbox-sandbox boundaries live in validate_pr_local.ps1.

    The module exists mainly to make the *honesty* rules testable. A local
    validation run posts a comment that maintainers treat as evidence, so the
    rules that decide "pass" vs "incomplete" vs "fail" must be pinned by tests,
    not re-derived by eye on every run.
#>

Set-StrictMode -Version Latest

# Ordered worst-last. Used by Get-WorstStatus to fold per-leg verdicts.
$script:StatusRank = @{
    pass       = 0
    incomplete = 1
    fail       = 2
    error      = 3
}

# The three live GUT hosts tools\run_all_tests.ps1 runs by default. Expressed
# with forward slashes to match the stage names it emits ("gut:tests/godot/gdk").
$script:DefaultRequiredHosts = @('tests/godot/gdk', 'tests/godot/playfab', 'tests/godot/gameinput')

function Get-StatusRank {
    param([Parameter(Mandatory = $true)][string]$Status)
    if (-not $script:StatusRank.ContainsKey($Status)) {
        throw "Unknown validation status '$Status'. Expected one of: $($script:StatusRank.Keys -join ', ')."
    }
    return $script:StatusRank[$Status]
}

function Get-WorstStatus {
    <#
    .SYNOPSIS
        Fold a set of statuses to the worst one ('pass' for an empty set is a
        caller error, so an empty set yields 'error').
    #>
    param([AllowEmptyCollection()][string[]]$Status = @())
    if ($null -eq $Status -or @($Status).Count -eq 0) { return 'error' }
    $worst = 'pass'
    foreach ($s in $Status) {
        if ((Get-StatusRank $s) -gt (Get-StatusRank $worst)) { $worst = $s }
    }
    return $worst
}

function Get-ExitCodeForStatus {
    <#
    .SYNOPSIS
        0 = fully green, 2 = ran but coverage is incomplete, 1 = failed/errored.
    .DESCRIPTION
        'incomplete' deliberately gets its own non-zero code: a run that skipped
        live coverage must never be scripted as a success, but it is also not
        the same signal as a real test failure.
    #>
    param([Parameter(Mandatory = $true)][string]$Status)
    switch ($Status) {
        'pass' { return 0 }
        'incomplete' { return 2 }
        default { return 1 }
    }
}

function Test-GitSha {
    param([Parameter(Mandatory = $true)][AllowEmptyString()][AllowNull()][string]$Sha)
    if ([string]::IsNullOrWhiteSpace($Sha)) { return $false }
    return [bool]([regex]::IsMatch($Sha, '^[0-9a-f]{40}$'))
}

function Test-SandboxId {
    <#
    .SYNOPSIS
        True for an Xbox sandbox id we are willing to pass to XblPCSandbox.
    .DESCRIPTION
        The id reaches a process argument, so it is restricted to the documented
        shapes: 'RETAIL' or '<5-8 alphanumerics>.<digits>' (e.g. lykhvw.0).
    #>
    param([Parameter(Mandatory = $true)][AllowEmptyString()][AllowNull()][string]$SandboxId)
    if ([string]::IsNullOrWhiteSpace($SandboxId)) { return $false }
    if ($SandboxId -ieq 'RETAIL') { return $true }
    return [bool]([regex]::IsMatch($SandboxId, '^[A-Za-z0-9]{4,10}\.[0-9]{1,3}$'))
}

function Get-SandboxIdFromOutput {
    <#
    .SYNOPSIS
        Extract the current sandbox id from `XblPCSandbox.exe` output.
    .DESCRIPTION
        Returns $null when nothing recognizable is present. The caller must
        treat $null as "unknown" and refuse to switch, rather than guessing
        RETAIL -- guessing would silently strand the machine in a test sandbox.
    #>
    param([AllowNull()][AllowEmptyString()][string]$Output)
    if ([string]::IsNullOrWhiteSpace($Output)) { return $null }
    foreach ($line in ($Output -split "`r?`n")) {
        $m = [regex]::Match($line, '(?i)\b(RETAIL|[A-Za-z0-9]{4,10}\.[0-9]{1,3})\b')
        if ($m.Success -and (Test-SandboxId $m.Groups[1].Value)) {
            return $m.Groups[1].Value
        }
    }
    return $null
}

function Get-GrdkEdition {
    <#
    .SYNOPSIS
        Parse `_GRDK_EDITION` out of the resolved grdk.h text.
    .DESCRIPTION
        This is the only trustworthy statement of which GDK the candidate build
        actually compiled against. addons\godot_gdk\src\gdk_edition.h falls back
        to 999999 when the macro is absent, and matrix membership in
        .github\gdk-versions.json proves nothing about the restored package, so
        neither may be used as evidence.
    #>
    param([AllowNull()][AllowEmptyString()][string]$HeaderText)
    if ([string]::IsNullOrWhiteSpace($HeaderText)) { return $null }
    $m = [regex]::Match($HeaderText, '(?m)^\s*#\s*define\s+_GRDK_EDITION\s+(\d{6})\s*$')
    if (-not $m.Success) { return $null }
    return $m.Groups[1].Value
}

function Get-MsGdkPackageVersion {
    <#
    .SYNOPSIS
        Read the restored ms-gdk port version from a parsed vcpkg.spdx.json.
    #>
    param([AllowNull()]$Spdx)
    if ($null -eq $Spdx) { return $null }
    if (-not ($Spdx.PSObject.Properties.Name -contains 'packages')) { return $null }
    foreach ($pkg in @($Spdx.packages)) {
        if ([string]$pkg.name -eq 'ms-gdk' -and -not [string]::IsNullOrWhiteSpace([string]$pkg.versionInfo)) {
            return [string]$pkg.versionInfo
        }
    }
    return $null
}

function Get-SummaryStage {
    param(
        [Parameter(Mandatory = $true)]$Summary,
        [Parameter(Mandatory = $true)][string]$Name
    )
    if ($null -eq $Summary) { return $null }
    if (-not ($Summary.PSObject.Properties.Name -contains 'stages')) { return $null }
    foreach ($s in @($Summary.stages)) {
        if ([string]$s.name -eq $Name) { return $s }
    }
    return $null
}

function Get-SummaryStagesByPrefix {
    <#
    .SYNOPSIS
        Return every stage whose name is $Prefix or begins with "$Prefix:".
    .DESCRIPTION
        run_all_tests.ps1 emits one bootstrap stage per discovered script
        ("bootstrap:tests/godot/gdk:gdk_bootstrap") but collapses an absent or
        empty directory into a single skip record named for the host alone
        ("bootstrap:tests/godot/gdk"). Both shapes must be found by one lookup,
        and a prefix match must not let "bootstrap:tests/godot/gdk2" satisfy a
        requirement for "bootstrap:tests/godot/gdk".
    #>
    param(
        [Parameter(Mandatory = $true)][AllowNull()]$Summary,
        [Parameter(Mandatory = $true)][string]$Prefix
    )
    $found = @()
    if ($null -eq $Summary) { return $found }
    if (-not ($Summary.PSObject.Properties.Name -contains 'stages')) { return $found }
    foreach ($s in @($Summary.stages)) {
        $name = [string]$s.name
        if ($name -eq $Prefix -or $name.StartsWith("${Prefix}:", [StringComparison]::Ordinal)) {
            $found += $s
        }
    }
    return $found
}

function Get-StageNumber {
    <#
    .SYNOPSIS
        Read one numeric counter out of a run-summary record.
    .DESCRIPTION
        An absent or null counter is legitimate -- the orchestrator leaves the
        per-stage tallies null for stages that do not count tests -- and is
        reported as $null for the caller to interpret.

        A counter that is present but not a number means the file is not the
        shape this tool knows how to read. That is raised as a descriptive
        error naming the stage, field, and offending value, rather than being
        left as a bare cast failure or quietly coerced to zero.
    #>
    param([AllowNull()]$Stage, [Parameter(Mandatory = $true)][string]$Field)
    if ($null -eq $Stage) { return $null }
    if (-not ($Stage.PSObject.Properties.Name -contains $Field)) { return $null }
    $v = $Stage.$Field
    if ($null -eq $v) { return $null }
    try {
        return [int]$v
    } catch {
        $owner = 'run-summary.json'
        if ($Stage.PSObject.Properties['name']) { $owner = "stage '$([string]$Stage.name)' in run-summary.json" }
        throw "$owner reported a non-numeric '$Field' value '$v'."
    }
}

function Get-SummaryFlag {
    <#
    .SYNOPSIS
        Read one boolean field out of a run-summary record, strictly.
    .DESCRIPTION
        PowerShell's [bool] cast treats every non-empty string as true, so a
        summary whose schema drifted to "live": "false" would read as a live
        run. The flags that gate the live-coverage claims are exactly where
        that must not happen, so anything other than a real JSON boolean is
        raised as an uninterpretable file rather than coerced.
    #>
    param([Parameter(Mandatory = $true)]$Summary, [Parameter(Mandatory = $true)][string]$Field)
    $v = $Summary.$Field
    if ($v -is [bool]) { return [bool]$v }
    $shown = if ($null -eq $v) { '<null>' } else { [string]$v }
    throw "run-summary.json reported a non-boolean '$Field' value '$shown'."
}

function New-EngineLegError {
    <#
    .SYNOPSIS
        Build the 'error' leg object used whenever a leg cannot be interpreted.
    .DESCRIPTION
        Every caller downstream -- manifest, report, and PR comment -- reads the
        same field set, so an uninterpretable leg must still return all of them.
        Keeping the shape in one place stops the failure paths from drifting
        away from the success path.
    #>
    param([Parameter(Mandatory = $true)][string[]]$Reasons)
    return [pscustomobject]@{
        Status       = 'error'
        Reasons      = @($Reasons)
        Gaps         = @()
        Tests        = 0
        Passing      = 0
        Failing      = 0
        Pending      = 0
        MpTotal      = $null
        MpPassed     = $null
        MpFailed     = $null
        MpSkipped    = $null
        DurationMs   = 0
        GodotVersion = 'unknown'
        XboxIdentity = 'unreported'
    }
}

function Get-EngineLegVerdict {
    <#
    .SYNOPSIS
        Turn one engine leg's run-summary.json into an honest verdict, and
        never throw while doing it.

    .DESCRIPTION
        See Get-EngineLegVerdictCore for the grading rules. This wrapper exists
        because the caller has no fallback: the verdict is computed after the
        tests have already run, and an exception escaping here would skip the
        manifest, the report, and the PR comment for a run whose live writes
        have already happened. A summary that is valid JSON can still be
        uninterpretable -- a non-numeric counter, a 'stages' value that is not
        a list of records -- so any failure to read it is preserved as an
        'error' leg instead, which outranks every other status and fails the
        run loudly.
    #>
    param(
        [Parameter(Mandatory = $true)][AllowNull()]$Summary,
        [AllowNull()]$OrchestratorResult,
        [string[]]$RequiredHosts = $script:DefaultRequiredHosts,
        [switch]$RequireOrchestrator,
        [switch]$RequireDoctest,
        [int]$OrchestratorExitCode = 0
    )

    try {
        return Get-EngineLegVerdictCore -Summary $Summary -OrchestratorResult $OrchestratorResult `
            -RequiredHosts $RequiredHosts -RequireOrchestrator:$RequireOrchestrator `
            -RequireDoctest:$RequireDoctest -OrchestratorExitCode $OrchestratorExitCode
    } catch {
        return New-EngineLegError -Reasons @("run-summary.json could not be interpreted: $($_.Exception.Message)")
    }
}

function Get-EngineLegVerdictCore {
    <#
    .SYNOPSIS
        Turn one engine leg's run-summary.json into an honest verdict.

    .DESCRIPTION
        run_all_tests.ps1's own exit code is necessary but not sufficient
        evidence for a live-write claim:

          - A default (unfiltered) PlayFab Multiplayer run is allowed to pass
            with zero passed scenarios, so the orchestrator stage can be green
            while covering nothing.
          - A live GUT host can report every test as pending (skipped) when the
            live gate is not actually reached, and still exit 0.
          - A signed-out machine is a specific case of that: the Xbox-backed
            suites mark a missing Xbox identity pending while their contract
            tests still pass, so a positive host tally alone does not prove
            signed-in coverage. run-summary.json carries that evidence only on
            candidates whose orchestrator emits 'xbox_signed_in', so it is
            graded when present and reported as unverified when absent --
            grading absence would permanently mark every branch that predates
            the field incomplete for evidence it cannot produce.
          - A leg invoked without -Live/-AllowLiveWrites would exit 0 having
            validated none of what this tool claims to validate.
          - run_all_tests.ps1 derives overall_status from failures only, so a
            skipped C++ doctest or a host with no bootstrap runners leaves the
            run green while this tool's report claims both were exercised.
          - A stage's status field and its tallies are separate evidence from
            the same file. A host reporting a nonzero 'failing' count fails the
            leg on its own, so a stale or inconsistent status='pass' cannot put
            a PASS heading above a nonzero Failed column.

        Those cases are reported as 'incomplete', never as 'pass'.

        The exit code is still necessary evidence, and it is the one signal the
        summary file cannot carry, so callers must pass it as
        -OrchestratorExitCode. Anything nonzero fails the leg outright.

    .OUTPUTS
        PSCustomObject: Status, Reasons, Gaps, Tests, Passing, Failing, Pending,
        DurationMs, GodotVersion, XboxIdentity.
    #>
    param(
        [Parameter(Mandatory = $true)][AllowNull()]$Summary,
        [AllowNull()]$OrchestratorResult,
        [string[]]$RequiredHosts = $script:DefaultRequiredHosts,
        [switch]$RequireOrchestrator,
        [switch]$RequireDoctest,
        [int]$OrchestratorExitCode = 0
    )

    $reasons = [System.Collections.Generic.List[string]]::new()
    $gaps = [System.Collections.Generic.List[string]]::new()

    if ($null -eq $Summary) {
        return New-EngineLegError -Reasons @('No run-summary.json was produced for this engine leg.')
    }

    $names = @($Summary.PSObject.Properties.Name)
    foreach ($required in @('overall_status', 'stages', 'live', 'live_writes')) {
        if ($names -notcontains $required) {
            return New-EngineLegError -Reasons @("run-summary.json is missing the '$required' field; refusing to interpret it.")
        }
    }

    $status = 'pass'

    if ([string]$Summary.overall_status -ne 'pass') {
        $status = 'fail'
        [void]$reasons.Add("run_all_tests reported overall_status='$($Summary.overall_status)'.")
    }

    # Read as strict booleans. [bool] on a string is true for anything
    # non-empty, so a summary carrying "live": "false" would otherwise read as
    # a live run and manufacture exactly the evidence these two checks exist to
    # demand. Anything that is not a JSON boolean is an uninterpretable file.
    $live = Get-SummaryFlag -Summary $Summary -Field 'live'
    $liveWrites = Get-SummaryFlag -Summary $Summary -Field 'live_writes'
    if (-not $live) {
        $status = 'error'
        [void]$reasons.Add('The leg did not run with -Live; it cannot support a live-coverage claim.')
    } elseif (-not $liveWrites) {
        $status = 'error'
        [void]$reasons.Add('The leg did not run with -AllowLiveWrites; it cannot support a live-write claim.')
    }

    # Signed-in Xbox identity, graded only when the candidate reports it. The
    # Xbox-backed suites mark a missing identity pending and still pass, so the
    # host tallies below cannot distinguish a signed-in run from a signed-out
    # one. 'xbox_signed_in' is an optional field for that reason: a candidate
    # that predates it has no way to answer, and failing those runs would mean
    # no pull request could ever be validated until the orchestrator change
    # merged. A candidate that does answer is held to its answer.
    $xboxIdentity = 'unreported'
    if ($names -contains 'xbox_signed_in') {
        if (Get-SummaryFlag -Summary $Summary -Field 'xbox_signed_in') {
            $xboxIdentity = 'signed-in'
        } else {
            $xboxIdentity = 'signed-out'
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add('The leg ran without a signed-in Xbox identity, so its Xbox-backed tests were pending rather than covered.')
        }
    }

    $tests = 0; $passing = 0; $failing = 0; $pending = 0

    foreach ($h in $RequiredHosts) {
        $stageName = "gut:$h"
        $stage = Get-SummaryStage -Summary $Summary -Name $stageName
        if ($null -eq $stage) {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add("$stageName did not run.")
            continue
        }
        $sStatus = [string]$stage.status
        if ($sStatus -eq 'skip') {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add("$stageName was skipped.")
            continue
        }
        if ($sStatus -ne 'pass') {
            $status = Get-WorstStatus @($status, 'fail')
            [void]$reasons.Add("$stageName status='$sStatus'.")
        }

        $t = Get-StageNumber -Stage $stage -Field 'tests'
        $p = Get-StageNumber -Stage $stage -Field 'passing'
        $f = Get-StageNumber -Stage $stage -Field 'failing'
        $n = Get-StageNumber -Stage $stage -Field 'pending'
        # The stage status is a single field written by the run being graded;
        # the tallies beside it are independent evidence. A host that reports
        # failures has failures regardless of what that field says, and the
        # report would otherwise print a PASS heading above a nonzero Failed
        # column. Graded before the test-count branch so a failure is never
        # swallowed by a host that also reported no discovered tests.
        if ($null -ne $f -and $f -lt 0) {
            $status = Get-WorstStatus @($status, 'error')
            [void]$reasons.Add("$stageName reported a negative failing count ($f), which is not a result this tool can read.")
        } elseif ($null -ne $f -and $f -gt 0) {
            $status = Get-WorstStatus @($status, 'fail')
            if ($sStatus -eq 'pass') {
                [void]$reasons.Add("$stageName reported $f failing test(s) despite status='pass'.")
            } else {
                [void]$reasons.Add("$stageName reported $f failing test(s).")
            }
        }
        if ($null -eq $t -or $t -le 0) {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add("$stageName discovered no tests.")
            if ($null -ne $f) { $failing += $f }
        } else {
            $tests += $t
            if ($null -ne $p) { $passing += $p }
            if ($null -ne $f) { $failing += $f }
            if ($null -ne $n) { $pending += $n }
            # A host that discovered tests but reported no passing count cannot
            # support a coverage claim: the report would print a zero against a
            # PASS heading. Absent is as unconfirmed as zero here.
            if ($null -eq $p) {
                $status = Get-WorstStatus @($status, 'incomplete')
                [void]$gaps.Add("$stageName reported no passing count, so its coverage cannot be confirmed.")
            } elseif ($p -le 0) {
                $status = Get-WorstStatus @($status, 'incomplete')
                [void]$gaps.Add("$stageName passed no tests (all skipped or risky).")
            }
            # Same rule for the failure tally. A host that ran tests and then
            # reported no failing count has not shown that nothing failed, and
            # the report's Failed column would print a zero it cannot stand
            # behind.
            if ($null -eq $f) {
                $status = Get-WorstStatus @($status, 'incomplete')
                [void]$gaps.Add("$stageName reported no failing count, so a clean result cannot be confirmed.")
            }
        }
    }

    $orchestrator = Get-SummaryStage -Summary $Summary -Name 'playfab-multiplayer-orchestrator'
    $mpPassed = $null; $mpTotal = $null; $mpFailed = $null; $mpSkipped = $null
    if ($RequireOrchestrator) {
        if ($null -eq $orchestrator -or [string]$orchestrator.status -eq 'skip') {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add('playfab-multiplayer-orchestrator did not run.')
        } else {
            if ([string]$orchestrator.status -ne 'pass') {
                $status = Get-WorstStatus @($status, 'fail')
                [void]$reasons.Add("playfab-multiplayer-orchestrator status='$($orchestrator.status)'.")
            }
            # run_all_tests only enforces "passed > 0" for an explicitly filtered
            # run, so an unfiltered orchestrator leg can be green having passed
            # nothing. Treat that as a coverage gap, not a success.
            #
            # The orchestrator does not populate the per-stage test counters --
            # they are null even on a run that executed 69 scenarios. Its real
            # tallies live in mp-test-results.json, so prefer those and fall
            # back to the stage only when that file is unavailable. Reading the
            # null stage counter as "passed nothing" would report a false
            # coverage gap on a run that actually covered plenty.
            $op = $null
            if ($null -ne $OrchestratorResult) {
                $sumProp = $OrchestratorResult.PSObject.Properties['summary']
                if ($sumProp -and $null -ne $sumProp.Value) {
                    $mpSummary = $sumProp.Value
                    $op = Get-StageNumber -Stage $mpSummary -Field 'passed'
                    $mpTotal = Get-StageNumber -Stage $mpSummary -Field 'total'
                    $mpFailed = Get-StageNumber -Stage $mpSummary -Field 'failed'
                    $mpSkipped = Get-StageNumber -Stage $mpSummary -Field 'skipped'
                }
            }
            if ($null -eq $op) { $op = Get-StageNumber -Stage $orchestrator -Field 'passing' }
            $mpPassed = $op
            if ($null -eq $op) {
                $status = Get-WorstStatus @($status, 'incomplete')
                [void]$gaps.Add('playfab-multiplayer-orchestrator reported no scenario counts, so its coverage cannot be confirmed.')
            } elseif ($op -le 0) {
                $status = Get-WorstStatus @($status, 'incomplete')
                [void]$gaps.Add('playfab-multiplayer-orchestrator passed no scenarios.')
            }
        }
    }

    $parse = Get-SummaryStage -Summary $Summary -Name 'parse-gate'
    if ($null -eq $parse -or [string]$parse.status -eq 'skip') {
        $status = Get-WorstStatus @($status, 'incomplete')
        [void]$gaps.Add('parse-gate did not run.')
    } elseif ([string]$parse.status -ne 'pass') {
        # overall_status should already carry this, but it is a single field
        # written by the same run; grading the stage directly means a stale or
        # inconsistent summary cannot leave a failed parse gate inside a
        # passing leg.
        $status = Get-WorstStatus @($status, 'fail')
        [void]$reasons.Add("parse-gate status='$($parse.status)'.")
    }

    # The native suite runs once, on the first engine leg; later legs are
    # invoked with -SkipDoctest on purpose, so only the first leg may assert it.
    if ($RequireDoctest) {
        $doctest = Get-SummaryStage -Summary $Summary -Name 'cpp-doctest'
        if ($null -eq $doctest -or [string]$doctest.status -eq 'skip') {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add('cpp-doctest did not run on the first engine leg, so no leg covered the native tests.')
        } elseif ([string]$doctest.status -ne 'pass') {
            $status = Get-WorstStatus @($status, 'fail')
            [void]$reasons.Add("cpp-doctest status='$($doctest.status)'.")
        }
    }

    # Bootstrap runners are per host and per script. An absent or empty
    # tests\bootstrap\ directory collapses to a single 'skip' record, which
    # overall_status ignores -- that is exactly the silent coverage loss this
    # has to surface.
    foreach ($h in $RequiredHosts) {
        $prefix = "bootstrap:$h"
        $stages = @(Get-SummaryStagesByPrefix -Summary $Summary -Prefix $prefix)
        if ($stages.Count -eq 0) {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add("$prefix did not run.")
            continue
        }
        $ran = 0
        foreach ($stage in $stages) {
            $sStatus = [string]$stage.status
            if ($sStatus -eq 'skip') { continue }
            $ran++
            if ($sStatus -ne 'pass') {
                $status = Get-WorstStatus @($status, 'fail')
                [void]$reasons.Add("$([string]$stage.name) status='$sStatus'.")
            }
        }
        if ($ran -eq 0) {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add("$prefix ran no bootstrap suites.")
        }
    }

    # The orchestrator's own exit code is the one piece of evidence the summary
    # file cannot contain. A leg that crashed, was cancelled, or failed after
    # writing an otherwise green summary must never settle as 'pass'.
    if ($OrchestratorExitCode -ne 0) {
        $status = Get-WorstStatus @($status, 'fail')
        [void]$reasons.Add("run_all_tests.ps1 exited $OrchestratorExitCode.")
    }

    $duration = 0
    $durationValue = Get-StageNumber -Stage $Summary -Field 'total_duration_ms'
    if ($null -ne $durationValue) { $duration = $durationValue }
    $godot = 'unknown'
    if ($names -contains 'godot_version' -and -not [string]::IsNullOrWhiteSpace([string]$Summary.godot_version)) {
        $godot = [string]$Summary.godot_version
    }

    return [pscustomobject]@{
        Status       = $status
        Reasons      = @($reasons)
        Gaps         = @($gaps)
        Tests        = $tests
        Passing      = $passing
        Failing      = $failing
        Pending      = $pending
        MpTotal      = $mpTotal
        MpPassed     = $mpPassed
        MpFailed     = $mpFailed
        MpSkipped    = $mpSkipped
        DurationMs   = $duration
        GodotVersion = $godot
        XboxIdentity = $xboxIdentity
    }
}

function ConvertTo-MarkdownCell {
    <#
    .SYNOPSIS
        Make arbitrary text safe inside a Markdown table cell.
    .DESCRIPTION
        Escapes pipes (which would otherwise invent new columns) and flattens
        newlines. Backslash-escaping must come first so an escaped pipe is not
        double-escaped.
    #>
    param([AllowNull()][AllowEmptyString()][string]$Text)
    if ($null -eq $Text) { return '' }
    $t = $Text -replace '\\', '\\'
    $t = $t -replace '\|', '\|'
    $t = $t -replace "`r`n", ' ' -replace "`r", ' ' -replace "`n", ' '
    return $t.Trim()
}

function Format-FencedBlock {
    <#
    .SYNOPSIS
        Render untrusted text inside a code fence that cannot be escaped.
    .DESCRIPTION
        Two failure modes this guards against:
          1. Content containing a backtick run that closes the fence early --
             solved by choosing a fence longer than the longest run inside.
          2. Truncation cutting the text off and losing the closing fence --
             solved by truncating the *content* and emitting the terminator
             unconditionally afterwards.
    #>
    param(
        [AllowNull()][AllowEmptyString()][string]$Text,
        [int]$MaxChars = 4000,
        [string]$Language = ''
    )
    $body = if ($null -eq $Text) { '' } else { $Text }
    $body = $body -replace "`r`n", "`n" -replace "`r", "`n"
    if ($body.Length -gt $MaxChars) {
        $body = $body.Substring(0, $MaxChars).TrimEnd() + "`n... (truncated)"
    }

    $longestRun = 0
    foreach ($m in [regex]::Matches($body, '`+')) {
        if ($m.Value.Length -gt $longestRun) { $longestRun = $m.Value.Length }
    }
    $fence = '`' * ([Math]::Max(3, $longestRun + 1))

    $lang = ''
    if ($Language -match '^[A-Za-z0-9_+-]{1,16}$') { $lang = $Language }

    return "$fence$lang`n$body`n$fence"
}

function Get-StatusLabel {
    param([Parameter(Mandatory = $true)][string]$Status)
    switch ($Status) {
        'pass' { return 'PASS' }
        'incomplete' { return 'INCOMPLETE' }
        'fail' { return 'FAIL' }
        default { return 'ERROR' }
    }
}

function Format-ValidationComment {
    <#
    .SYNOPSIS
        Render the PR comment body from a validation manifest object.
    .DESCRIPTION
        The comment is evidence, so it states exactly what ran, against which
        commit and which SDK, and lists every coverage gap. A run that is not
        fully green says so in its heading.
    #>
    param([Parameter(Mandatory = $true)]$Manifest)

    $status = [string]$Manifest.status
    $lines = [System.Collections.Generic.List[string]]::new()

    [void]$lines.Add("<!-- pr-local-validation -->")
    [void]$lines.Add("## Local PR validation - $(Get-StatusLabel $status)")
    [void]$lines.Add('')

    $debugProp = $Manifest.PSObject.Properties['debug_build_status']
    $debugFailed = $debugProp -and $debugProp.Value -ne 'pass'
    [void]$lines.Add('Results apply only to the exact commit, SDK and selected engines below, not to the entire PR support matrix or a later commit.')
    $selected = @($Manifest.engines_selected)
    [void]$lines.Add('')
    [void]$lines.Add("- **Selected Godot versions**: $(($selected | ForEach-Object { '`' + (ConvertTo-MarkdownCell ([string]$_)) + '`' }) -join ', ')")
    if ($debugFailed) { [void]$lines.Add('- Debug preparation failed. Live tests and Release were not run; no live-test coverage is claimed.') }
    $notRun = @($Manifest.engines_not_run)
    if ($notRun.Count -gt 0) {
        [void]$lines.Add("- **Selected engines not completed**: $(($notRun | ForEach-Object { ConvertTo-MarkdownCell ([string]$_) }) -join ', ')")
    }
    [void]$lines.Add('')
    [void]$lines.Add("- **Head commit**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.head_sha))``")
    [void]$lines.Add("- **Started (UTC)**: $(ConvertTo-MarkdownCell ([string]$Manifest.started_at))")
    [void]$lines.Add("- **Finished (UTC)**: $(ConvertTo-MarkdownCell ([string]$Manifest.finished_at))")
    $edition = if ($Manifest.gdk_edition) { [string]$Manifest.gdk_edition } else { 'unavailable' }
    $package = if ($Manifest.ms_gdk_version) { [string]$Manifest.ms_gdk_version } else { 'unavailable' }
    [void]$lines.Add("- **GDK edition (resolved ``grdk.h``)**: ``$(ConvertTo-MarkdownCell $edition)``")
    $gdkPinnedProp = $Manifest.PSObject.Properties['gdk_pinned']
    $gdkOrigin = if ($gdkPinnedProp -and $gdkPinnedProp.Value) { 'pinned by request' } else { 'inherited from the candidate checkout' }
    [void]$lines.Add("- **ms-gdk package (restored by vcpkg)**: ``$(ConvertTo-MarkdownCell $package)`` ($gdkOrigin)")
    [void]$lines.Add("- **Declared default GDK (informational)**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.gdk_declared_default))``")
    [void]$lines.Add("- **PlayFab title**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.playfab_title_id))``")
    $restoredProp = $Manifest.PSObject.Properties['sandbox_restored']
    $attemptedProp = $Manifest.PSObject.Properties['sandbox_attempted']
    if ($attemptedProp -and -not $attemptedProp.Value) {
        [void]$lines.Add('- **Xbox sandbox**: not changed (live execution was not reached)')
    } elseif ($restoredProp -and -not $restoredProp.Value) {
        [void]$lines.Add("- **Xbox sandbox**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_id))`` (**NOT restored** to ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_restored_to))`` afterwards -- this machine is still in the test sandbox)")
    } else {
        [void]$lines.Add("- **Xbox sandbox**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_id))`` (restored to ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_restored_to))`` afterwards)")
    }
    if ($debugProp) { [void]$lines.Add("- **Debug build**: ``$(ConvertTo-MarkdownCell ([string]$debugProp.Value))``") }
    [void]$lines.Add("- **Release build**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.release_build_status))``")
    [void]$lines.Add('')
    [void]$lines.Add('| Godot | Status | GUT tests | Passed | Failed | Pending | Multiplayer scenarios | Duration |')
    [void]$lines.Add('|-------|--------|-----------|--------|--------|---------|-----------------------|----------|')
    foreach ($leg in @($Manifest.engines)) {
        $secs = [Math]::Round(([int]$leg.duration_ms) / 1000.0, 1)
        $mpCell = 'not reported'
        $mpTotalProp = $leg.PSObject.Properties['mp_total']
        $mpPassedProp = $leg.PSObject.Properties['mp_passed']
        if ($mpTotalProp -and $null -ne $mpTotalProp.Value -and $mpPassedProp -and $null -ne $mpPassedProp.Value) {
            $mpCell = '{0}/{1} passed' -f [int]$mpPassedProp.Value, [int]$mpTotalProp.Value
            $mpFailedProp = $leg.PSObject.Properties['mp_failed']
            if ($mpFailedProp -and $null -ne $mpFailedProp.Value -and [int]$mpFailedProp.Value -gt 0) {
                $mpCell += ', {0} failed' -f [int]$mpFailedProp.Value
            }
        }
        [void]$lines.Add(('| `{0}` | {1} | {2} | {3} | {4} | {5} | {6} | {7}s |' -f `
            (ConvertTo-MarkdownCell ([string]$leg.version)),
            (Get-StatusLabel ([string]$leg.status)),
            [int]$leg.tests, [int]$leg.passing, [int]$leg.failing, [int]$leg.pending,
            (ConvertTo-MarkdownCell $mpCell), $secs))
    }
    [void]$lines.Add('')

    $allReasons = [System.Collections.Generic.List[string]]::new()
    $allGaps = [System.Collections.Generic.List[string]]::new()
    foreach ($leg in @($Manifest.engines)) {
        foreach ($r in @($leg.reasons)) { [void]$allReasons.Add("``$($leg.version)``: $r") }
        foreach ($g in @($leg.gaps)) { [void]$allGaps.Add("``$($leg.version)``: $g") }
    }
    foreach ($r in @($Manifest.reasons)) { [void]$allReasons.Add([string]$r) }
    foreach ($g in @($Manifest.gaps)) { [void]$allGaps.Add([string]$g) }

    if ($allReasons.Count -gt 0) {
        [void]$lines.Add('### Failures')
        [void]$lines.Add('')
        foreach ($r in $allReasons) { [void]$lines.Add("- $(ConvertTo-MarkdownCell $r)") }
        [void]$lines.Add('')
    }
    if ($allGaps.Count -gt 0) {
        [void]$lines.Add('### Coverage gaps')
        [void]$lines.Add('')
        foreach ($g in $allGaps) { [void]$lines.Add("- $(ConvertTo-MarkdownCell $g)") }
        [void]$lines.Add('')
    }

    [void]$lines.Add('### Not covered by this run')
    [void]$lines.Add('')
    [void]$lines.Add('- Console (Xbox hardware) targets: this validates the PC/GDK desktop target only.')
    [void]$lines.Add('- Packaging and submission flows.')
    [void]$lines.Add('- Any PlayFab title other than the one named above.')
    [void]$lines.Add('- Any other GDK version; this run does not assess the entire support list.')
    $skippedEngines = @($Manifest.engines_skipped)
    if ($skippedEngines.Count -gt 0) {
        [void]$lines.Add("- Unselected Godot versions: $(($skippedEngines | ForEach-Object { ConvertTo-MarkdownCell ([string]$_) }) -join ', ').")
    }
    if (@($Manifest.engines).Count -gt 0) {
        [void]$lines.Add('- The C++ doctest binary is requested on the first engine leg only; see leg results for completion.')
    }
    # A signed-out machine still tallies green, because the Xbox-backed suites
    # mark a missing identity pending. Legs whose summary did not answer are
    # named here rather than graded, so the reader is never told a run proved
    # signed-in coverage that nothing in its evidence establishes.
    $identityUnreported = @(@($Manifest.engines) | Where-Object {
            $p = $_.PSObject.Properties['xbox_identity']
            (-not $p) -or ([string]$p.Value -ne 'signed-in' -and [string]$p.Value -ne 'signed-out')
        })
    if ($identityUnreported.Count -gt 0) {
        $unreportedVersions = @($identityUnreported | ForEach-Object { '`' + (ConvertTo-MarkdownCell ([string]$_.version)) + '`' })
        [void]$lines.Add(("- Signed-in Xbox identity on {0}: that run_all_tests.ps1 does not report it, so Xbox-backed tests may have been marked pending rather than run." -f ($unreportedVersions -join ', ')))
    }
    [void]$lines.Add('')
    $rerun = "pwsh -File tools/validate_pr_local.ps1 -PullRequest $([int]$Manifest.pull_request) -AllowLiveWrites"
    $rerun += " -GodotVersion $($selected -join ',')"
    if ($gdkPinnedProp -and $gdkPinnedProp.Value) {
        $rerun += " -GdkVersion $(ConvertTo-MarkdownCell ([string]$Manifest.gdk_requested))"
    }
    [void]$lines.Add("Generated by ``tools/validate_pr_local.ps1``. Run this selection again with ``$rerun`` (resolves the PR head anew).")

    return ($lines -join "`n")
}

function Select-EngineMatrix {
    <#
    .SYNOPSIS
        Resolve which Godot versions a run will cover.
    .DESCRIPTION
        With no request the full supported matrix runs. A request must name
        versions the candidate actually declares: silently accepting an unknown
        version would either run nothing or run an unpinned engine, and both
        look like a narrower pass rather than an error.

        Selection preserves the manifest's order so legs and reports stay
        comparable between runs, and a narrowed run is reported as such so it
        can never be mistaken for full-matrix evidence.

        Requests are split on commas because the wrapper is launched with
        'pwsh -File', which binds '-GodotVersion a,b' as one string rather than
        two elements. The report's own reproduction command prints that form, so
        rejecting it would make the documented copy-paste path fail.
    #>
    param(
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][string[]]$Supported,
        [string[]]$Requested
    )

    $all = @($Supported | Where-Object { $_ })
    if ($all.Count -eq 0) { throw 'The candidate declares no supported Godot versions.' }

    $wanted = @($Requested | ForEach-Object { ([string]$_).Split(',') } | ForEach-Object { $_.Trim() } | Where-Object { $_ })
    if ($wanted.Count -eq 0) {
        return [pscustomobject]@{ Selected = $all; Skipped = @(); Narrowed = $false }
    }

    $unknown = @($wanted | Where-Object { $v = $_; -not ($all | Where-Object { $_ -ieq $v }) })
    if ($unknown.Count -gt 0) {
        throw ("Godot version(s) '{0}' are not in this pull request's supported matrix. Choose from: {1}." -f ($unknown -join ', '), ($all -join ', '))
    }

    $selected = @($all | Where-Object { $v = $_; $wanted | Where-Object { $_ -ieq $v } })
    $skipped = @($all | Where-Object { $selected -notcontains $_ })
    return [pscustomobject]@{
        Selected = $selected
        Skipped  = $skipped
        Narrowed = ($skipped.Count -gt 0)
    }
}

function Select-GdkVersion {
    <#
    .SYNOPSIS
        Resolve which GDK the run will build against.
    .DESCRIPTION
        One run builds exactly one GDK, so this picks a single version rather
        than a matrix: a GDK leg means a full reconfigure plus the whole engine
        matrix again, which is hours of live writes per edition.

        Without a request the candidate's declared default is used. That is only
        an expectation, not a guarantee -- the local default build resolves
        'ms-gdk' from vcpkg.json and the registry baseline, not from
        .github/gdk-versions.json -- so the caller must still compare the
        report the actual restored package, not this declared default.

        A request must name a version the candidate declares as supported.
        Accepting an unknown version would pin an SDK the pull request never
        claimed to support and report it as validated support.
    #>
    param(
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][object[]]$Supported,
        [string]$Default,
        [string]$Requested
    )

    $entries = @()
    foreach ($entry in @($Supported | Where-Object { $_ })) {
        $version = if ($entry -is [string]) { $entry } else { [string]$entry.version }
        if (-not $version) { continue }
        $edition = $null
        if ($entry -isnot [string] -and $entry.PSObject.Properties['edition']) {
            $edition = [string]$entry.edition
        }
        $entries += [pscustomobject]@{ Version = $version; Edition = $edition }
    }
    if ($entries.Count -eq 0) { throw 'The candidate declares no supported GDK versions in .github/gdk-versions.json.' }

    $all = @($entries | ForEach-Object { $_.Version })

    if ($Requested) {
        $match = @($entries | Where-Object { $_.Version -ieq $Requested })
        if ($match.Count -eq 0) {
            throw ("GDK version '{0}' is not in this pull request's supported list. Choose from: {1}." -f $Requested, ($all -join ', '))
        }
        $chosen = $match[0]
        $pinned = $true
    }
    else {
        if (-not $Default) { throw 'The candidate declares no default GDK version in .github/gdk-versions.json.' }
        $match = @($entries | Where-Object { $_.Version -ieq $Default })
        $chosen = if ($match.Count -gt 0) { $match[0] } else { [pscustomobject]@{ Version = $Default; Edition = $null } }
        $pinned = $false
    }

    return [pscustomobject]@{
        Version   = $chosen.Version
        Edition   = $chosen.Edition
        Pinned    = $pinned
    }
}

function New-MsGdkOverride {
    <#
    .SYNOPSIS
        Build the vcpkg manifest 'overrides' entry that pins ms-gdk.
    .DESCRIPTION
        Mirrors .github/actions/build-addons/action.yml so a locally pinned
        build resolves the same dependency CI would resolve. A blank version
        means "use the registry baseline", which is the unpinned default and
        must not be written as an override.
    #>
    param([Parameter(Mandatory = $true)][string]$Version)
    if (-not ($Version -match '^[0-9]+(\.[0-9]+)+$')) {
        throw "Refusing to pin ms-gdk to a non-version string '$Version'."
    }
    return [pscustomobject]@{ name = 'ms-gdk'; version = $Version }
}

function Get-RunVerdict {
    <#
    .SYNOPSIS
        Grade only the selected work, builds and sandbox restoration.
    .OUTPUTS
        One of 'pass', 'incomplete', 'fail', 'error'.
    #>
    param(
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][string[]]$LegStatuses,
        [Parameter(Mandatory = $true)][string]$ReleaseStatus,
        [bool]$ExecutionFailed = $false,
        [bool]$SandboxRestoreFailed = $false,
        [ValidateSet('pass', 'fail', 'error')][string]$DebugStatus = 'pass'
    )
    $statuses = @($LegStatuses)
    if ($DebugStatus -ne 'pass') { $statuses += $DebugStatus }
    if ($ReleaseStatus -ne 'pass' -and $ReleaseStatus -ne 'not run') { $statuses += 'fail' }
    if ($DebugStatus -eq 'pass' -and $ReleaseStatus -eq 'not run') { $statuses += 'incomplete' }
    if ($ExecutionFailed) { $statuses += 'error' }
    if ($SandboxRestoreFailed) { $statuses += 'error' }
    # No legs at all is not a pass: nothing was measured.
    if ($LegStatuses.Count -eq 0 -and $DebugStatus -eq 'pass') { $statuses += 'error' }
    return Get-WorstStatus $statuses
}

function New-RunDirectoryName {
    <#
    .SYNOPSIS
        Deterministic, collision-resistant run directory name.
    #>
    param(
        [Parameter(Mandatory = $true)][int]$PullRequest,
        [Parameter(Mandatory = $true)][string]$HeadSha,
        [Parameter(Mandatory = $true)][datetime]$TimestampUtc
    )
    if (-not (Test-GitSha $HeadSha)) {
        throw "Refusing to build a run directory name from a non-SHA head '$HeadSha'."
    }
    if ($PullRequest -le 0) {
        throw "Pull request number must be positive; got $PullRequest."
    }
    return ('pr-{0}-{1}-{2}' -f $PullRequest, $HeadSha.Substring(0, 12), $TimestampUtc.ToString('yyyyMMddTHHmmssZ'))
}

Export-ModuleMember -Function @(
    'Get-StatusRank',
    'Get-WorstStatus',
    'Get-ExitCodeForStatus',
    'Test-GitSha',
    'Test-SandboxId',
    'Get-SandboxIdFromOutput',
    'Get-GrdkEdition',
    'Get-MsGdkPackageVersion',
    'Get-SummaryStage',
    'Get-SummaryStagesByPrefix',
    'Get-StageNumber',
    'Get-SummaryFlag',
    'Get-EngineLegVerdict',
    'ConvertTo-MarkdownCell',
    'Format-FencedBlock',
    'Get-StatusLabel',
    'Format-ValidationComment',
    'Select-EngineMatrix',
    'Select-GdkVersion',
    'New-MsGdkOverride',
    'Get-RunVerdict',
    'New-RunDirectoryName'
)
