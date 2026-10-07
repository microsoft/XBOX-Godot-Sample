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
        $Stages
    )
    if ($null -eq $Stages) {
        $Stages = @(
            (New-Stage -Name 'parse-gate'),
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
        stages = @($Stages)
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
} finally {
    Remove-Item -LiteralPath $fixtureRoot -Recurse -Force -ErrorAction SilentlyContinue
}

Write-Host ''
Write-Host "$script:Passed passed, $script:Failed failed." -ForegroundColor $(if ($script:Failed -eq 0) { 'Green' } else { 'Red' })
exit ([int]($script:Failed -gt 0))
