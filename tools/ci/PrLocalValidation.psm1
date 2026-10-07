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
    param([AllowNull()]$Stage, [Parameter(Mandatory = $true)][string]$Field)
    if ($null -eq $Stage) { return $null }
    if (-not ($Stage.PSObject.Properties.Name -contains $Field)) { return $null }
    $v = $Stage.$Field
    if ($null -eq $v) { return $null }
    return [int]$v
}

function Get-EngineLegVerdict {
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
          - A leg invoked without -Live/-AllowLiveWrites would exit 0 having
            validated none of what this tool claims to validate.
          - run_all_tests.ps1 derives overall_status from failures only, so a
            skipped C++ doctest or a host with no bootstrap runners leaves the
            run green while this tool's report claims both were exercised.

        Those cases are reported as 'incomplete', never as 'pass'.

        The exit code is still necessary evidence, and it is the one signal the
        summary file cannot carry, so callers must pass it as
        -OrchestratorExitCode. Anything nonzero fails the leg outright.

    .OUTPUTS
        PSCustomObject: Status, Reasons, Gaps, Tests, Passing, Failing, Pending,
        DurationMs, GodotVersion.
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
        return [pscustomobject]@{
            Status = 'error'; Reasons = @('No run-summary.json was produced for this engine leg.')
            Gaps = @(); Tests = 0; Passing = 0; Failing = 0; Pending = 0
            MpTotal = $null; MpPassed = $null; MpFailed = $null; MpSkipped = $null
            DurationMs = 0; GodotVersion = 'unknown'
        }
    }

    $names = @($Summary.PSObject.Properties.Name)
    foreach ($required in @('overall_status', 'stages', 'live', 'live_writes')) {
        if ($names -notcontains $required) {
            return [pscustomobject]@{
                Status = 'error'; Reasons = @("run-summary.json is missing the '$required' field; refusing to interpret it.")
                Gaps = @(); Tests = 0; Passing = 0; Failing = 0; Pending = 0
            MpTotal = $null; MpPassed = $null; MpFailed = $null; MpSkipped = $null
                DurationMs = 0; GodotVersion = 'unknown'
            }
        }
    }

    $status = 'pass'

    if ([string]$Summary.overall_status -ne 'pass') {
        $status = 'fail'
        [void]$reasons.Add("run_all_tests reported overall_status='$($Summary.overall_status)'.")
    }

    if (-not [bool]$Summary.live) {
        $status = 'error'
        [void]$reasons.Add('The leg did not run with -Live; it cannot support a live-coverage claim.')
    } elseif (-not [bool]$Summary.live_writes) {
        $status = 'error'
        [void]$reasons.Add('The leg did not run with -AllowLiveWrites; it cannot support a live-write claim.')
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
        if ($null -eq $t -or $t -le 0) {
            $status = Get-WorstStatus @($status, 'incomplete')
            [void]$gaps.Add("$stageName discovered no tests.")
        } else {
            $tests += $t
            if ($null -ne $p) { $passing += $p }
            if ($null -ne $f) { $failing += $f }
            if ($null -ne $n) { $pending += $n }
            if ($null -ne $p -and $p -le 0) {
                $status = Get-WorstStatus @($status, 'incomplete')
                [void]$gaps.Add("$stageName passed no tests (all skipped or risky).")
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
    if ($names -contains 'total_duration_ms' -and $null -ne $Summary.total_duration_ms) {
        $duration = [int]$Summary.total_duration_ms
    }
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

    $skippedProp = $Manifest.PSObject.Properties['engines_skipped']
    $skippedEngines = @()
    if ($skippedProp) { $skippedEngines = @($skippedProp.Value | Where-Object { $_ }) }

    if ($skippedEngines.Count -gt 0) {
        $ran = @(@($Manifest.engines) | ForEach-Object { '`' + (ConvertTo-MarkdownCell ([string]$_.version)) + '`' })
        $skip = @($skippedEngines | ForEach-Object { '`' + (ConvertTo-MarkdownCell ([string]$_)) + '`' })
        [void]$lines.Add("Ran ``tools/run_all_tests.ps1 -Live -AllowLiveWrites`` against a fresh checkout of this pull request's head commit.")
        [void]$lines.Add('')
        [void]$lines.Add("> **The Godot matrix was narrowed by request.** This run covered $($ran -join ', ') and skipped $($skip -join ', '). It is not full-matrix evidence.")
    } else {
        [void]$lines.Add("Ran ``tools/run_all_tests.ps1 -Live -AllowLiveWrites`` against a fresh checkout of this pull request's head commit, on every Godot version in the candidate checkout's ``.github/godot-versions.json``.")
    }
    [void]$lines.Add('')
    [void]$lines.Add("- **Head commit**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.head_sha))``")
    [void]$lines.Add("- **Started (UTC)**: $(ConvertTo-MarkdownCell ([string]$Manifest.started_at))")
    [void]$lines.Add("- **Finished (UTC)**: $(ConvertTo-MarkdownCell ([string]$Manifest.finished_at))")
    [void]$lines.Add("- **GDK edition (resolved ``grdk.h``)**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.gdk_edition))``")
    $gdkPinnedProp = $Manifest.PSObject.Properties['gdk_pinned']
    $gdkOrigin = if ($gdkPinnedProp -and $gdkPinnedProp.Value) { 'pinned by request' } else { 'inherited from the candidate checkout' }
    [void]$lines.Add("- **ms-gdk package (restored by vcpkg)**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.ms_gdk_version))`` ($gdkOrigin)")
    [void]$lines.Add("- **PlayFab title**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.playfab_title_id))``")
    [void]$lines.Add("- **Xbox sandbox**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_id))`` (restored to ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_restored_to))`` afterwards)")
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
    [void]$lines.Add('- The C++ doctest binary is engine-independent, so it ran on the first engine leg only.')
    [void]$lines.Add('')
    $rerun = "pwsh -File tools/validate_pr_local.ps1 -PullRequest $([int]$Manifest.pull_request) -AllowLiveWrites"
    if ($skippedEngines.Count -gt 0) {
        $ranVersions = @(@($Manifest.engines) | ForEach-Object { ConvertTo-MarkdownCell ([string]$_.version) })
        $rerun += " -GodotVersion $($ranVersions -join ',')"
    }
    if ($gdkPinnedProp -and $gdkPinnedProp.Value) {
        $rerun += " -GdkVersion $(ConvertTo-MarkdownCell ([string]$Manifest.ms_gdk_version))"
    }
    [void]$lines.Add("Generated by ``tools/validate_pr_local.ps1``. Reproduce this exact scope with ``$rerun``.")

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
    #>
    param(
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][string[]]$Supported,
        [string[]]$Requested
    )

    $all = @($Supported | Where-Object { $_ })
    if ($all.Count -eq 0) { throw 'The candidate declares no supported Godot versions.' }

    $wanted = @($Requested | Where-Object { $_ })
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
        restored package against the Expected value this returns.

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
        Expected  = $chosen.Version
        Edition   = $chosen.Edition
        Pinned    = $pinned
        Supported = $all
        Uncovered = @($all | Where-Object { $_ -ine $chosen.Version })
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

function Get-GdkCoverageGap {
    <#
    .SYNOPSIS
        Report GDK editions a GDK-touching pull request left unvalidated.
    .DESCRIPTION
        Every run leaves the non-default supported editions unbuilt, so
        reporting that unconditionally would make the gap list meaningless
        noise. It only matters when the pull request itself edits the support
        list: then the uncovered entries are exactly the support claims this
        run did not exercise.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Selected,
        [Parameter(Mandatory = $true)][AllowEmptyCollection()][string[]]$Uncovered,
        [Parameter(Mandatory = $true)][bool]$ManifestChanged,
        [Parameter(Mandatory = $true)][int]$PullRequest
    )
    $rest = @($Uncovered | Where-Object { $_ })
    if (-not $ManifestChanged -or $rest.Count -eq 0) { return $null }
    return ("This pull request changes ``.github/gdk-versions.json``, and one run builds one GDK. Validated ``{0}``; still unvalidated: {1}. Re-run per edition with ``-PullRequest {2} -GdkVersion <version>``." -f
        $Selected,
        (($rest | ForEach-Object { '`' + $_ + '`' }) -join ', '),
        $PullRequest)
}

function Get-GdkIdentityGap {
    <#
    .SYNOPSIS
        Report when the build resolved a different GDK than the candidate declares.
    .DESCRIPTION
        .github/gdk-versions.json drives CI, which injects an explicit override.
        A plain 'cmake --preset default' does not read it at all, so a pull
        request can advance the declared default while the local build quietly
        keeps resolving the old SDK from the registry baseline. Reporting the
        restored version alone is accurate but easy to misread as validating
        the new default, so the mismatch is called out explicitly.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Expected,
        [Parameter(Mandatory = $true)][string]$Restored,
        [Parameter(Mandatory = $true)][bool]$Pinned
    )
    if ($Pinned -or $Expected -ieq $Restored) { return $null }
    return ("The candidate declares default GDK ``{0}`` but the build resolved ``{1}``. The local default build resolves ``ms-gdk`` from ``vcpkg.json`` and the registry baseline, not from ``.github/gdk-versions.json``. Re-run with ``-GdkVersion {0}`` to pin it." -f $Expected, $Restored)
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
    'Get-EngineLegVerdict',
    'ConvertTo-MarkdownCell',
    'Format-FencedBlock',
    'Get-StatusLabel',
    'Format-ValidationComment',
    'Select-EngineMatrix',
    'Select-GdkVersion',
    'New-MsGdkOverride',
    'Get-GdkCoverageGap',
    'Get-GdkIdentityGap',
    'New-RunDirectoryName'
)
