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

        Those cases are reported as 'incomplete', never as 'pass'.

    .OUTPUTS
        PSCustomObject: Status, Reasons, Gaps, Tests, Passing, Failing, Pending,
        DurationMs, GodotVersion.
    #>
    param(
        [Parameter(Mandatory = $true)][AllowNull()]$Summary,
        [string[]]$RequiredHosts = $script:DefaultRequiredHosts,
        [switch]$RequireOrchestrator
    )

    $reasons = [System.Collections.Generic.List[string]]::new()
    $gaps = [System.Collections.Generic.List[string]]::new()

    if ($null -eq $Summary) {
        return [pscustomobject]@{
            Status = 'error'; Reasons = @('No run-summary.json was produced for this engine leg.')
            Gaps = @(); Tests = 0; Passing = 0; Failing = 0; Pending = 0
            DurationMs = 0; GodotVersion = 'unknown'
        }
    }

    $names = @($Summary.PSObject.Properties.Name)
    foreach ($required in @('overall_status', 'stages', 'live', 'live_writes')) {
        if ($names -notcontains $required) {
            return [pscustomobject]@{
                Status = 'error'; Reasons = @("run-summary.json is missing the '$required' field; refusing to interpret it.")
                Gaps = @(); Tests = 0; Passing = 0; Failing = 0; Pending = 0
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
            $op = Get-StageNumber -Stage $orchestrator -Field 'passing'
            if ($null -eq $op -or $op -le 0) {
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
    [void]$lines.Add("Ran ``tools/run_all_tests.ps1 -Live -AllowLiveWrites`` against a fresh checkout of this pull request's head commit, on every Godot version in the candidate checkout's ``.github/godot-versions.json``.")
    [void]$lines.Add('')
    [void]$lines.Add("- **Head commit**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.head_sha))``")
    [void]$lines.Add("- **Started (UTC)**: $(ConvertTo-MarkdownCell ([string]$Manifest.started_at))")
    [void]$lines.Add("- **Finished (UTC)**: $(ConvertTo-MarkdownCell ([string]$Manifest.finished_at))")
    [void]$lines.Add("- **GDK edition (resolved ``grdk.h``)**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.gdk_edition))``")
    [void]$lines.Add("- **ms-gdk package (restored by vcpkg)**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.ms_gdk_version))``")
    [void]$lines.Add("- **PlayFab title**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.playfab_title_id))``")
    [void]$lines.Add("- **Xbox sandbox**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_id))`` (restored to ``$(ConvertTo-MarkdownCell ([string]$Manifest.sandbox_restored_to))`` afterwards)")
    [void]$lines.Add("- **Release build**: ``$(ConvertTo-MarkdownCell ([string]$Manifest.release_build_status))``")
    [void]$lines.Add('')
    [void]$lines.Add('| Godot | Status | Tests | Passed | Failed | Skipped | Duration |')
    [void]$lines.Add('|-------|--------|-------|--------|--------|---------|----------|')
    foreach ($leg in @($Manifest.engines)) {
        $secs = [Math]::Round(([int]$leg.duration_ms) / 1000.0, 1)
        [void]$lines.Add(('| `{0}` | {1} | {2} | {3} | {4} | {5} | {6}s |' -f `
            (ConvertTo-MarkdownCell ([string]$leg.version)),
            (Get-StatusLabel ([string]$leg.status)),
            [int]$leg.tests, [int]$leg.passing, [int]$leg.failing, [int]$leg.pending, $secs))
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
    [void]$lines.Add("Generated by ``tools/validate_pr_local.ps1``. Re-run locally with ``pwsh -File tools/validate_pr_local.ps1 -PullRequest $([int]$Manifest.pull_request) -AllowLiveWrites``.")

    return ($lines -join "`n")
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
    'Get-StageNumber',
    'Get-EngineLegVerdict',
    'ConvertTo-MarkdownCell',
    'Format-FencedBlock',
    'Get-StatusLabel',
    'Format-ValidationComment',
    'New-RunDirectoryName'
)
