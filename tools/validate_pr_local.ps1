#Requires -Version 7.0
<#
.SYNOPSIS
    Clone a pull request at its exact head commit, build it, and run the full
    live-write test suite on every supported Godot version.

.DESCRIPTION
    This is the trusted wrapper. It runs from a checkout you already trust and
    never sources logic from the candidate pull request: the only things taken
    from the PR are its source tree (built in an isolated directory) and its
    declared Godot matrix.

    What a run does, in order:

      1. Preflight (tooling, auth, sandbox reachability, single-run lock).
      2. Resolve the PR's head commit via `gh` and pin to that exact SHA. The
         merge result is deliberately not used -- reviewers need evidence about
         the commit they are looking at.
      3. Clone into a fresh run directory and check the SHA out detached, with
         submodules.
      4. Build Debug once, then prove which GDK was actually restored by reading
         the resolved grdk.h and the vcpkg ms-gdk package metadata.
      5. Download each Godot version named by the candidate's
         .github/godot-versions.json into a run-private engine directory, and
         stage the GDK/PlayFab redistributable DLLs next to each Godot
         executable (xgameruntime.dll resolves against the *Godot* process
         directory, not the extension's).
      6. Switch the machine's Xbox sandbox, run tools/run_all_tests.ps1 with
         -Live -AllowLiveWrites once per engine, then always restore the
         original sandbox.
      7. Build Release (after the live matrix -- builds mirror binaries into
         shared addon and test-host directories).
      8. Write a validation manifest plus a human-readable report, and post the
         result as a PR comment.

    Exit codes: 0 fully green, 2 ran but coverage was incomplete, 1 failed.

.PARAMETER PullRequest
    Pull request number in the upstream repository.

.PARAMETER AllowLiveWrites
    Required to actually run. Without it the script performs preflight and the
    checkout/build phases only, and refuses to touch live services.

.PARAMETER DryRun
    Resolve metadata, print the exact plan, and exit without creating anything.

.PARAMETER WorkRoot
    Parent directory for run directories. Defaults to
    $env:LOCALAPPDATA\godot-gdk-pr-validation.

.PARAMETER GodotVersion
    Narrow the engine matrix to the named Godot version(s), which must appear in
    the candidate's .github/godot-versions.json supported list. A narrowed run is
    reported as incomplete coverage (exit 2) even when every leg passes, because
    it is not the full-matrix evidence the unqualified report claims.

.PARAMETER GdkVersion
    Pin the build to a specific ms-gdk version, which must appear in the
    candidate's .github/gdk-versions.json supported list. Without it the build
    resolves ms-gdk from the candidate's vcpkg.json and registry baseline, which
    is not necessarily the default that manifest declares. Only one GDK is built
    per run: a second GDK means a full reconfigure plus the whole engine matrix
    again, which is hours of additional live writes.

.PARAMETER NoComment
    Produce the report but do not post it to the pull request.

.EXAMPLE
    pwsh -NoLogo -NoProfile -File .\tools\validate_pr_local.ps1 -PullRequest 202 -DryRun

.EXAMPLE
    pwsh -NoLogo -NoProfile -File .\tools\validate_pr_local.ps1 -PullRequest 202 -AllowLiveWrites

.EXAMPLE
    # A GDK-only bump: the native SDK surface does not vary by engine, so one
    # engine is usually enough to justify the change.
    pwsh -NoLogo -NoProfile -File .\tools\validate_pr_local.ps1 -PullRequest 202 -AllowLiveWrites -GodotVersion 4.6.1-stable

.EXAMPLE
    # Validate a supported edition the pull request adds without promoting it to
    # default, which the plain build would otherwise never resolve.
    pwsh -NoLogo -NoProfile -File .\tools\validate_pr_local.ps1 -PullRequest 202 -AllowLiveWrites -GdkVersion 2604.2.7849
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [ValidateRange(1, 999999)]
    [int]$PullRequest,

    [switch]$AllowLiveWrites,
    [switch]$DryRun,
    [string]$WorkRoot,
    [string[]]$GodotVersion,
    [string]$GdkVersion,
    [switch]$NoComment
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# --- Fixed, non-secret profile -------------------------------------------------
# These are deliberately constants rather than parameters. Live writes land in a
# single already-provisioned sandbox title; making them configurable would turn a
# typo into writes against an unrelated title.
$script:Repository           = 'microsoft/XBOX-Godot-Sample'
$script:RepositoryUrl        = 'https://github.com/microsoft/XBOX-Godot-Sample.git'
$script:PlayFabTitleId       = '10D176'
$script:PlayFabCustomId      = 'godot-gdk-ext-live-smoke'
$script:PlayFabMatchmaking   = 'godot_gdk_ext_live_smoke_queue'
$script:SandboxId            = 'lykhvw.0'
$script:LockName             = 'Global\godot-gdk-pr-local-validation'

# Removed from the environment of every candidate-controlled child process:
# the CMake configure/build, the orchestrator, and everything they spawn.
#
# This is not a security sandbox and does not pretend to be one. The candidate
# still runs as this user and can read the same credential stores, gh's own
# config, and the keyring. What it does do is stop *this wrapper* from handing
# a live PlayFab developer secret or a GitHub token to code it is measuring,
# and stop an operator's ambient LIVE_* flags from reaching tests that are
# supposed to be gated by this script's explicit arguments. GitHub credentials
# are restored for the trusted metadata and comment calls, which never run
# candidate code.
$script:ScrubbedEnvironment = @(
    'PLAYFAB_DEVELOPER_SECRET_KEY',
    'GITHUB_TOKEN',
    'GH_TOKEN',
    'GITHUB_ENTERPRISE_TOKEN',
    'GH_ENTERPRISE_TOKEN',
    'LIVE_TESTS',
    'LIVE_WRITE_TESTS'
)

$script:ToolsRoot = Split-Path -Parent $PSCommandPath
$script:CiRoot    = Join-Path $script:ToolsRoot 'ci'

Import-Module (Join-Path $script:CiRoot 'PrLocalValidation.psm1') -Force
Import-Module (Join-Path $script:CiRoot 'GodotAcquisition.psm1') -Force

# --- Small helpers -------------------------------------------------------------

function Write-Phase {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host ''
    Write-Host "=== $Message" -ForegroundColor Cyan
}

function Write-Note {
    param([Parameter(Mandatory = $true)][string]$Message)
    Write-Host "    $Message" -ForegroundColor DarkGray
}

function Test-CommandAvailable {
    param([Parameter(Mandatory = $true)][string]$Name)
    return [bool](Get-Command $Name -CommandType Application, ExternalScript -ErrorAction SilentlyContinue)
}

function Test-IsElevated {
    $id = [Security.Principal.WindowsIdentity]::GetCurrent()
    return ([Security.Principal.WindowsPrincipal]::new($id)).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Invoke-Logged {
    <#
    .SYNOPSIS
        Run a native command, tee its combined output to a log, and return the
        exit code without throwing.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$FilePath,
        [Parameter(Mandatory = $true)][string[]]$ArgumentList,
        [Parameter(Mandatory = $true)][string]$LogPath,
        [string]$WorkingDirectory,
        [switch]$ScrubCredentials
    )
    $logDir = Split-Path -Parent $LogPath
    if (-not (Test-Path -LiteralPath $logDir)) { New-Item -ItemType Directory -Force -Path $logDir | Out-Null }

    $previous = Get-Location
    if ($WorkingDirectory) { Set-Location -LiteralPath $WorkingDirectory }
    # & inherits this process's environment block at launch, so removing the
    # variables here is what keeps them out of the child. They go back in the
    # finally so the wrapper's own trusted gh calls still work.
    $savedEnv = @{}
    if ($ScrubCredentials) {
        foreach ($name in $script:ScrubbedEnvironment) {
            $savedEnv[$name] = [Environment]::GetEnvironmentVariable($name)
            if ($null -ne $savedEnv[$name]) { Remove-Item "Env:\$name" -ErrorAction SilentlyContinue }
        }
    }
    try {
        Write-Note "$FilePath $($ArgumentList -join ' ')"
        # Out-Host keeps the teed output off this function's own pipeline so the
        # caller receives the exit code and nothing else.
        & $FilePath @ArgumentList 2>&1 | Tee-Object -FilePath $LogPath -Append | Out-Host
        return $LASTEXITCODE
    } finally {
        foreach ($name in $savedEnv.Keys) {
            if ($null -ne $savedEnv[$name]) { Set-Item "Env:\$name" -Value $savedEnv[$name] }
        }
        Set-Location -LiteralPath $previous
    }
}

function Invoke-GitChecked {
    param(
        [Parameter(Mandatory = $true)][string[]]$ArgumentList,
        [string]$WorkingDirectory,
        [Parameter(Mandatory = $true)][string]$LogPath
    )
    $code = Invoke-Logged -FilePath 'git' -ArgumentList $ArgumentList -LogPath $LogPath -WorkingDirectory $WorkingDirectory
    if ($code -ne 0) { throw "git $($ArgumentList -join ' ') failed with exit code $code." }
}

function Get-GitOutput {
    param(
        [Parameter(Mandatory = $true)][string[]]$ArgumentList,
        [Parameter(Mandatory = $true)][string]$WorkingDirectory
    )
    $previous = Get-Location
    Set-Location -LiteralPath $WorkingDirectory
    try {
        $out = & git @ArgumentList 2>&1
        if ($LASTEXITCODE -ne 0) { throw "git $($ArgumentList -join ' ') failed: $out" }
        return ($out | Out-String).Trim()
    } finally {
        Set-Location -LiteralPath $previous
    }
}

function Get-PullRequestMetadata {
    param([Parameter(Mandatory = $true)][int]$Number)
    $json = & gh pr view $Number --repo $script:Repository --json number,headRefOid,headRefName,baseRefName,state,isCrossRepository,title,url,files 2>&1
    if ($LASTEXITCODE -ne 0) {
        throw "gh pr view $Number failed: $($json | Out-String)"
    }
    $meta = ($json | Out-String) | ConvertFrom-Json
    if (-not (Test-GitSha ([string]$meta.headRefOid))) {
        throw "gh returned an unusable head commit '$($meta.headRefOid)' for PR $Number."
    }
    return $meta
}

# --- Xbox sandbox --------------------------------------------------------------

function Get-XblPCSandboxPath {
    $candidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft GDK\bin\XblPCSandbox.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft GDK\bin\XblPCSandbox.exe'),
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft GDK\Command Line Tools\XblPCSandbox.exe')
    )
    foreach ($c in $candidates) {
        if ($c -and (Test-Path -LiteralPath $c)) { return $c }
    }
    $onPath = Get-Command 'XblPCSandbox.exe' -ErrorAction SilentlyContinue
    if ($onPath) { return $onPath.Source }
    return $null
}

function Get-CurrentSandbox {
    param([Parameter(Mandatory = $true)][string]$ExePath)
    $out = & $ExePath '/get' 2>&1 | Out-String
    $id = Get-SandboxIdFromOutput -Output $out
    if (-not $id) {
        throw "Could not determine the current Xbox sandbox from XblPCSandbox output:`n$out"
    }
    return $id
}

function Set-CurrentSandbox {
    param(
        [Parameter(Mandatory = $true)][string]$ExePath,
        [Parameter(Mandatory = $true)][string]$SandboxId,
        [Parameter(Mandatory = $true)][string]$LogPath
    )
    if (-not (Test-SandboxId $SandboxId)) {
        throw "Refusing to pass '$SandboxId' to XblPCSandbox; it is not a recognized sandbox id."
    }
    # /noApps avoids terminating running Store apps, but it still restarts the
    # Xbox Live Auth Manager service, which is why elevation is required.
    $code = Invoke-Logged -FilePath $ExePath -ArgumentList @($SandboxId, '/noApps') -LogPath $LogPath
    if ($code -ne 0) { throw "XblPCSandbox $SandboxId failed with exit code $code." }

    $actual = Get-CurrentSandbox -ExePath $ExePath
    if ($actual -ine $SandboxId) {
        throw "Sandbox switch did not take effect: requested '$SandboxId', machine reports '$actual'."
    }
    return $actual
}

# --- Engine preparation --------------------------------------------------------

function Copy-RedistributableDlls {
    <#
    .SYNOPSIS
        Stage the vcpkg runtime DLLs next to a Godot executable.
    .DESCRIPTION
        xgameruntime.dll initialization fails with E_GAMERUNTIME_DLL_NOT_FOUND
        (0x89240101) unless the whole thunk chain is resolvable from the Godot
        *process* directory. The nightly live workflow does the same thing; here
        the destination is run-private so nothing is left behind on the machine.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$CheckoutRoot,
        [Parameter(Mandatory = $true)][string]$EngineDir
    )
    $sources = @(
        (Join-Path $CheckoutRoot 'build\vcpkg_installed\x64-windows\bin'),
        (Join-Path $CheckoutRoot 'build\vcpkg_installed\x64-windows\debug\bin')
    )
    $copied = 0
    foreach ($src in $sources) {
        if (-not (Test-Path -LiteralPath $src)) { continue }
        foreach ($dll in (Get-ChildItem -LiteralPath $src -Filter '*.dll' -File -ErrorAction SilentlyContinue)) {
            Copy-Item -LiteralPath $dll.FullName -Destination $EngineDir -Force
            $copied++
        }
    }
    if ($copied -eq 0) {
        throw "No redistributable DLLs were found under '$CheckoutRoot\build\vcpkg_installed'. The Debug build did not restore the GDK/PlayFab runtime, so live tests would fail misleadingly."
    }
    return $copied
}

function Test-PullRequestTouchesPath {
    <#
    .SYNOPSIS
        Did this pull request change the given repository-relative path?
    #>
    param(
        [Parameter(Mandatory = $true)]$Metadata,
        [Parameter(Mandatory = $true)][string]$Path
    )
    $filesProp = $Metadata.PSObject.Properties['files']
    if (-not $filesProp -or -not $filesProp.Value) { return $false }
    $needle = $Path.Replace('\', '/')
    foreach ($file in @($filesProp.Value)) {
        if (([string]$file.path).Replace('\', '/') -ieq $needle) { return $true }
    }
    return $false
}

function Read-GdkManifest {
    <#
    .SYNOPSIS
        Read the candidate checkout's declared GDK support list.
    #>
    param([Parameter(Mandatory = $true)][string]$ManifestPath)
    if (-not (Test-Path -LiteralPath $ManifestPath)) {
        throw "The candidate checkout has no GDK manifest at '$ManifestPath'."
    }
    $json = (Get-Content -LiteralPath $ManifestPath -Raw) | ConvertFrom-Json
    $default = $null
    if ($json.PSObject.Properties['default']) { $default = [string]$json.default }
    $supported = @()
    if ($json.PSObject.Properties['supported']) { $supported = @($json.supported) }
    return [pscustomobject]@{ Default = $default; Supported = $supported }
}

function Set-MsGdkOverride {
    <#
    .SYNOPSIS
        Pin ms-gdk in the candidate checkout's vcpkg manifest.
    .DESCRIPTION
        Mirrors .github/actions/build-addons/action.yml, which injects the same
        top-level 'overrides' entry before configuring. Editing the candidate's
        manifest is a deliberate, recorded deviation from its source: without it
        the build resolves whatever the registry baseline offers, so a requested
        edition could not be validated at all.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$CheckoutRoot,
        [Parameter(Mandatory = $true)][string]$Version
    )
    $manifestPath = Join-Path $CheckoutRoot 'vcpkg.json'
    $json = (Get-Content -LiteralPath $manifestPath -Raw) | ConvertFrom-Json

    $override = New-MsGdkOverride -Version $Version
    $existing = @()
    if ($json.PSObject.Properties['overrides']) {
        $existing = @($json.overrides | Where-Object { $_ -and ([string]$_.name) -ne 'ms-gdk' })
    }
    $json | Add-Member -NotePropertyName 'overrides' -NotePropertyValue (@($override) + $existing) -Force

    ($json | ConvertTo-Json -Depth 32) | Set-Content -LiteralPath $manifestPath -Encoding utf8
    return $manifestPath
}

function Get-SdkIdentity {
    <#
    .SYNOPSIS
        Prove which GDK the candidate build compiled against.
    #>
    param([Parameter(Mandatory = $true)][string]$CheckoutRoot)

    $headerPath = Join-Path $CheckoutRoot 'build\vcpkg_installed\x64-windows\include\grdk.h'
    $spdxPath   = Join-Path $CheckoutRoot 'build\vcpkg_installed\x64-windows\share\ms-gdk\vcpkg.spdx.json'

    $edition = $null
    if (Test-Path -LiteralPath $headerPath) {
        $edition = Get-GrdkEdition -HeaderText (Get-Content -LiteralPath $headerPath -Raw)
    }
    $package = $null
    if (Test-Path -LiteralPath $spdxPath) {
        $package = Get-MsGdkPackageVersion -Spdx ((Get-Content -LiteralPath $spdxPath -Raw) | ConvertFrom-Json)
    }

    if (-not $edition) {
        throw "Could not read _GRDK_EDITION from '$headerPath'. Without it there is no evidence of which GDK was used, and the 999999 fallback in gdk_edition.h must never be reported as proof."
    }
    if (-not $package) {
        throw "Could not read the restored ms-gdk version from '$spdxPath'."
    }

    return [pscustomobject]@{ Edition = $edition; Package = $package }
}

# --- Main ----------------------------------------------------------------------

$runStatus = 'error'
$exitCode = 1

try {
    Write-Phase "Preflight"

    if (-not $IsWindows) { throw 'This script validates the Windows/GDK target and must run on Windows.' }

    foreach ($tool in @('git', 'gh', 'cmake', 'pwsh')) {
        if (-not (Test-CommandAvailable $tool)) { throw "Required tool '$tool' was not found on PATH." }
    }
    Write-Note 'git, gh, cmake, pwsh found.'

    & gh auth status 2>&1 | Out-Null
    if ($LASTEXITCODE -ne 0) { throw "gh is not authenticated. Run 'gh auth login' first." }
    Write-Note 'gh is authenticated.'

    if (-not $WorkRoot) {
        $WorkRoot = Join-Path $env:LOCALAPPDATA 'godot-gdk-pr-validation'
    }
    $WorkRoot = [System.IO.Path]::GetFullPath($WorkRoot)

    Write-Phase "Resolving pull request #$PullRequest"
    $pr = Get-PullRequestMetadata -Number $PullRequest
    $headSha = [string]$pr.headRefOid
    Write-Note "Title:  $($pr.title)"
    Write-Note "Head:   $headSha ($($pr.headRefName))"
    Write-Note "Base:   $($pr.baseRefName)  State: $($pr.state)"
    if ([string]$pr.state -ne 'OPEN') {
        Write-Warning "PR #$PullRequest is $($pr.state). Validating it anyway, at the head commit above."
    }

    # This tool executes the candidate's CMake, PowerShell, native binaries and
    # GDScript with the operator's own account, GitHub credentials and signed-in
    # Xbox identity. The fresh clone is workspace isolation, not a security
    # boundary. A fork PR is therefore untrusted code and is refused outright --
    # there is no flag to override this, because an override would be used.
    if ([bool]$pr.isCrossRepository) {
        throw ("PR #$PullRequest comes from a fork ($($pr.headRefName)). This tool runs candidate " +
            'build and test code with your full account context and cannot sandbox it, so it only ' +
            'validates same-repository pull requests. Review the diff and re-push it to a branch in ' +
            "$script:Repository if you need to validate it locally.")
    }

    $startedAt = [datetime]::UtcNow
    $runName = New-RunDirectoryName -PullRequest $PullRequest -HeadSha $headSha -TimestampUtc $startedAt
    $runDir = Join-Path $WorkRoot $runName
    $checkoutDir = Join-Path $runDir 'checkout'
    $enginesDir = Join-Path $runDir 'engines'
    $resultsDir = Join-Path $runDir 'results'
    $logsDir = Join-Path $runDir 'logs'
    $cacheDir = Join-Path $WorkRoot 'archive-cache'

    if ($DryRun) {
        Write-Phase 'Dry run - planned actions'
        Write-Host "  Repository      : $script:Repository"
        Write-Host "  Head commit     : $headSha"
        Write-Host "  Run directory   : $runDir"
        Write-Host "  PlayFab title   : $script:PlayFabTitleId (custom id '$script:PlayFabCustomId')"
        Write-Host "  Xbox sandbox    : $script:SandboxId (current sandbox captured and restored)"
        Write-Host '  Live writes     : always - this tool has no read-only mode'
        Write-Host "  Confirmed       : $([bool]$AllowLiveWrites) (-AllowLiveWrites)"
        Write-Host "  Post comment    : $(-not [bool]$NoComment)"
        if ($GodotVersion) {
            Write-Host "  Engines         : narrowed by request to $($GodotVersion -join ', ') (validated against the candidate manifest; result will be reported as incomplete coverage)"
        } else {
            Write-Host '  Engines         : read from the candidate checkout .github/godot-versions.json'
        }
        if ($GdkVersion) {
            Write-Host "  GDK             : pinned by request to ms-gdk $GdkVersion (validated against the candidate manifest; an overrides entry is written into the checkout's vcpkg.json)"
        } else {
            Write-Host '  GDK             : whatever the candidate vcpkg.json and registry baseline resolve, compared against .github/gdk-versions.json default'
        }
        Write-Host ''
        Write-Host '  Also performed: Debug build, SDK identity read from the built vcpkg tree,'
        Write-Host '  sandbox switch and restore, and a Release build after the live matrix.'
        Write-Host ''
        Write-Host '  Per engine, inside the candidate checkout:'
        Write-Host "    pwsh -File tools/run_all_tests.ps1 -SkipBuild -Live -AllowLiveWrites ``"
        Write-Host "      -PlayFabTitleId $script:PlayFabTitleId -PlayFabCustomId $script:PlayFabCustomId ``"
        Write-Host "      -PlayFabMatchmakingQueue $script:PlayFabMatchmaking -OutDir <results>/<version>"
        Write-Host ''
        Write-Host '  Nothing was created. Re-run with -AllowLiveWrites to execute.'
        Write-Host ''

        # Report the two host preconditions that most often block a real run. Both
        # checks are read-only: locating the tool and asking the current process
        # about its own token. The sandbox is deliberately not queried, because
        # XblPCSandbox needs elevation and restarts the Xbox auth service.
        $dryExe = Get-XblPCSandboxPath
        if ($dryExe) {
            Write-Host "  Sandbox tool    : $dryExe"
        } else {
            Write-Warning 'XblPCSandbox.exe was not found. Install the Microsoft GDK command line tools before a real run.'
        }
        if (Test-IsElevated) {
            Write-Host '  Elevation       : elevated'
        } else {
            Write-Host "  Elevation       : not elevated - required unless this machine is already in sandbox $script:SandboxId"
        }
        exit 0
    }

    if (-not $AllowLiveWrites) {
        throw "Refusing to run: this tool exists to execute the live-write suite. Pass -AllowLiveWrites to confirm that writes against PlayFab title $script:PlayFabTitleId are intended, or use -DryRun to see the plan."
    }

    $sandboxExe = Get-XblPCSandboxPath
    if (-not $sandboxExe) { throw 'XblPCSandbox.exe was not found. Install the Microsoft GDK command line tools.' }
    Write-Note "Sandbox tool: $sandboxExe"

    $originalSandbox = Get-CurrentSandbox -ExePath $sandboxExe
    Write-Note "Current sandbox: $originalSandbox"
    if ($originalSandbox -ine $script:SandboxId -and -not (Test-IsElevated)) {
        throw "The machine is in sandbox '$originalSandbox' and must switch to '$script:SandboxId', which requires an elevated shell. Re-run this script as Administrator."
    }

    $lock = [System.Threading.Mutex]::new($false, $script:LockName)
    if (-not $lock.WaitOne(0)) {
        throw "Another local validation run is already in progress on this machine. Live writes target a single shared PlayFab title, so runs must not overlap."
    }
    Write-Warning "Live writes target the shared PlayFab title $script:PlayFabTitleId. Coordinate with the nightly 'playfab-live' workflow before continuing; this lock only covers this machine."

    try {
        if (Test-Path -LiteralPath $runDir) {
            throw "Run directory '$runDir' already exists. Runs are never reused so that evidence cannot be mixed between commits."
        }
        foreach ($d in @($runDir, $enginesDir, $resultsDir, $logsDir, $cacheDir)) {
            New-Item -ItemType Directory -Force -Path $d | Out-Null
        }
        Write-Note "Run directory: $runDir"

        # --- Checkout ----------------------------------------------------------
        Write-Phase "Cloning $script:Repository at $headSha"
        $gitLog = Join-Path $logsDir 'git.log'
        New-Item -ItemType Directory -Force -Path $checkoutDir | Out-Null
        Invoke-GitChecked -ArgumentList @('init', '-q') -WorkingDirectory $checkoutDir -LogPath $gitLog
        Invoke-GitChecked -ArgumentList @('remote', 'add', 'origin', $script:RepositoryUrl) -WorkingDirectory $checkoutDir -LogPath $gitLog
        Invoke-GitChecked -ArgumentList @('fetch', '--no-tags', 'origin', "+refs/pull/$PullRequest/head:refs/remotes/origin/pr/$PullRequest") -WorkingDirectory $checkoutDir -LogPath $gitLog

        $resolved = Get-GitOutput -ArgumentList @('rev-parse', '--verify', "$headSha^{commit}") -WorkingDirectory $checkoutDir
        if ($resolved -ne $headSha) {
            throw "Fetched ref does not contain head commit $headSha (git resolved '$resolved'). The PR may have been updated mid-run."
        }
        Invoke-GitChecked -ArgumentList @('checkout', '--detach', $headSha) -WorkingDirectory $checkoutDir -LogPath $gitLog
        Invoke-GitChecked -ArgumentList @('submodule', 'update', '--init', '--recursive') -WorkingDirectory $checkoutDir -LogPath $gitLog

        $checkedOut = Get-GitOutput -ArgumentList @('rev-parse', 'HEAD') -WorkingDirectory $checkoutDir
        if ($checkedOut -ne $headSha) { throw "Checkout verification failed: HEAD is $checkedOut, expected $headSha." }
        Write-Note "Verified HEAD = $headSha"

        # Resolve the engine matrix before the build: a mistyped -GodotVersion
        # should cost seconds, not a full Debug build.
        $manifestPath = Join-Path $checkoutDir '.github\godot-versions.json'
        $godotManifest = Read-GodotManifest -ManifestPath $manifestPath
        if (@($godotManifest.Supported).Count -eq 0) { throw "The candidate checkout declares no supported Godot versions in '$manifestPath'." }
        $matrix = Select-EngineMatrix -Supported $godotManifest.Supported -Requested $GodotVersion
        $versions = @($matrix.Selected)
        if ($matrix.Narrowed) {
            Write-Warning "Engine matrix narrowed to $($versions -join ', '); skipping $($matrix.Skipped -join ', '). This run cannot report full-matrix coverage."
        } else {
            Write-Note "Supported versions: $($versions -join ', ')"
        }

        # --- Debug build -------------------------------------------------------
        # Resolve the GDK before the build too, for the same reason: a mistyped
        # -GdkVersion must fail in seconds.
        $gdkManifestPath = Join-Path $checkoutDir '.github\gdk-versions.json'
        $gdkManifest = Read-GdkManifest -ManifestPath $gdkManifestPath
        $gdk = Select-GdkVersion -Supported $gdkManifest.Supported -Default $gdkManifest.Default -Requested $GdkVersion
        if ($gdk.Pinned) {
            $pinnedManifest = Set-MsGdkOverride -CheckoutRoot $checkoutDir -Version $gdk.Version
            Write-Warning "Pinned ms-gdk to $($gdk.Version) by writing an overrides entry into '$pinnedManifest'. The checkout no longer matches the pull request's vcpkg manifest exactly."
        } else {
            Write-Note "GDK (candidate default): $($gdk.Version)"
        }

        Write-Phase 'Building Debug'
        $buildLog = Join-Path $logsDir 'build-debug.log'
        $code = Invoke-Logged -FilePath 'cmake' -ArgumentList @('--preset', 'default') -LogPath $buildLog -WorkingDirectory $checkoutDir -ScrubCredentials
        if ($code -ne 0) { throw "cmake --preset default failed with exit code $code." }
        $code = Invoke-Logged -FilePath 'cmake' -ArgumentList @('--build', '--preset', 'debug') -LogPath $buildLog -WorkingDirectory $checkoutDir -ScrubCredentials
        if ($code -ne 0) { throw "cmake --build --preset debug failed with exit code $code." }

        $sdk = Get-SdkIdentity -CheckoutRoot $checkoutDir
        Write-Note "Resolved GDK edition : $($sdk.Edition)"
        Write-Note "Restored ms-gdk      : $($sdk.Package)"

        # A pinned run asked for one exact SDK. If vcpkg resolved a different one
        # the override did not take, and every result below would be attributed
        # to an SDK that was never built.
        if ($gdk.Pinned) {
            if ($sdk.Package -ine $gdk.Version) {
                throw "Requested ms-gdk $($gdk.Version) but the build restored $($sdk.Package). Refusing to report results against an SDK that was not the one requested."
            }
            # We know we got the requested package, so a differing edition means
            # the candidate's own version-to-edition mapping is wrong.
            if ($gdk.Edition -and $sdk.Edition -ine $gdk.Edition) {
                throw "The candidate maps ms-gdk $($gdk.Version) to edition $($gdk.Edition), but that package resolved edition $($sdk.Edition)."
            }
        }

        $gdkManifestChanged = Test-PullRequestTouchesPath -Metadata $pr -Path '.github/gdk-versions.json'
        $gdkIdentityGap = Get-GdkIdentityGap -Expected $gdk.Expected -Restored $sdk.Package -Pinned ([bool]$gdk.Pinned)
        $gdkCoverageGap = Get-GdkCoverageGap -Selected $sdk.Package -Uncovered $gdk.Uncovered -ManifestChanged $gdkManifestChanged -PullRequest $PullRequest
        if ($gdkIdentityGap) { Write-Warning $gdkIdentityGap }
        if ($gdkCoverageGap) { Write-Warning $gdkCoverageGap }

        # --- Engines -----------------------------------------------------------
        Write-Phase 'Preparing Godot engines'

        $engines = @()
        foreach ($version in $versions) {
            # -Force: the per-run engines directory is fresh, so there is nothing
            # to reuse. Saying so explicitly keeps the "never a shared extracted
            # engine directory" promise true even if a future -WorkRoot change
            # makes the path collide.
            $installJson = & pwsh -NoLogo -NoProfile -File (Join-Path $script:CiRoot 'get_godot.ps1') `
                -Mode Install -Version $version -ManifestPath $manifestPath `
                -DestinationRoot $enginesDir -ArchiveCacheDir $cacheDir -Force 2>&1
            if ($LASTEXITCODE -ne 0) { throw "Acquiring Godot $version failed: $($installJson | Out-String)" }
            $info = ($installJson | Select-Object -Last 1 | Out-String).Trim() | ConvertFrom-Json

            $engineDir = Split-Path -Parent $info.Path
            $dlls = Copy-RedistributableDlls -CheckoutRoot $checkoutDir -EngineDir $engineDir
            Write-Note "$version -> $($info.Path) (+$dlls runtime DLLs)"
            $engines += [pscustomobject]@{ Version = $version; Info = $info; EngineDir = $engineDir }
        }

        # --- Live matrix -------------------------------------------------------
        Write-Phase "Switching Xbox sandbox to $script:SandboxId"
        $restoreNote = Join-Path $runDir 'SANDBOX-RESTORE.txt'
        Set-Content -LiteralPath $restoreNote -Encoding utf8 -Value @"
This run changed the machine's Xbox sandbox.
Original sandbox: $originalSandbox
If the run was interrupted, restore it manually from an elevated shell:
    & "$sandboxExe" $originalSandbox /noApps
"@
        $sandboxLog = Join-Path $logsDir 'sandbox.log'
        if ($originalSandbox -ine $script:SandboxId) {
            # Switching restarts Xbox Live Auth Manager, which signs the Xbox app
            # out. The GDK live tiers need a test account signed in, and this
            # script cannot do that for you -- it is an interactive system UI.
            # So a mid-run switch usually produces a 'no_default_user' leg.
            Write-Warning "The machine is in '$originalSandbox', not '$script:SandboxId'. Switching now restarts Xbox Live Auth Manager and signs the Xbox app out, and the GDK live tiers need a signed-in test account. For a clean run, cancel, switch the sandbox, sign a $script:SandboxId test account into the Xbox app, then re-run; this script will then skip the switch entirely."
            Set-CurrentSandbox -ExePath $sandboxExe -SandboxId $script:SandboxId -LogPath $sandboxLog | Out-Null
            Write-Note "Sandbox is now $script:SandboxId"
        } else {
            Write-Note "Already in $script:SandboxId; no switch needed."
        }

        $legs = @()
        try {
            $first = $true
            foreach ($engine in $engines) {
                Write-Phase "Live run on Godot $($engine.Version)"
                $legOut = Join-Path $resultsDir $engine.Version
                New-Item -ItemType Directory -Force -Path $legOut | Out-Null

                $legArgs = @(
                    '-NoLogo', '-NoProfile', '-File', (Join-Path $checkoutDir 'tools\run_all_tests.ps1'),
                    '-SkipBuild', '-Live', '-AllowLiveWrites',
                    '-PlayFabTitleId', $script:PlayFabTitleId,
                    '-PlayFabCustomId', $script:PlayFabCustomId,
                    '-PlayFabMatchmakingQueue', $script:PlayFabMatchmaking,
                    '-OutDir', $legOut
                )
                if (-not $first) {
                    # The C++ doctest binary is engine-independent; running it
                    # once is evidence enough and keeps the matrix honest about
                    # what each leg adds.
                    $legArgs += '-SkipDoctest'
                }

                $saved = @{}
                $legCode = $null
                foreach ($name in @('GODOT', 'GODOT_BIN', 'GODOT_CONSOLE')) {
                    $saved[$name] = [Environment]::GetEnvironmentVariable($name)
                }
                try {
                    $env:GODOT = $engine.Info.Path
                    $env:GODOT_BIN = $engine.Info.Path
                    $env:GODOT_CONSOLE = $engine.Info.Path
                    $legLog = Join-Path $logsDir "run-$($engine.Version).log"
                    $legCode = Invoke-Logged -FilePath 'pwsh' -ArgumentList $legArgs -LogPath $legLog -WorkingDirectory $checkoutDir -ScrubCredentials
                } finally {
                    foreach ($name in $saved.Keys) {
                        if ($null -eq $saved[$name]) {
                            Remove-Item "Env:\$name" -ErrorAction SilentlyContinue
                        } else {
                            Set-Item "Env:\$name" -Value $saved[$name]
                        }
                    }
                }

                $summaryPath = Join-Path $legOut 'run-summary.json'
                $summary = $null
                $summaryError = $null
                if (Test-Path -LiteralPath $summaryPath) {
                    try {
                        $summary = (Get-Content -LiteralPath $summaryPath -Raw) | ConvertFrom-Json
                    } catch {
                        # A truncated or malformed summary must not abort the run:
                        # the leg still has to produce a verdict, a manifest, and a
                        # posted report that say why it could not be interpreted.
                        $summary = $null
                        $summaryError = "run-summary.json could not be parsed: $($_.Exception.Message)"
                        Write-Note $summaryError
                    }
                }
                $mpResultPath = Join-Path $legOut 'mp-orchestrator\mp-test-results.json'
                $mpResult = $null
                if (Test-Path -LiteralPath $mpResultPath) {
                    try {
                        $mpResult = (Get-Content -LiteralPath $mpResultPath -Raw) | ConvertFrom-Json
                    } catch {
                        Write-Note "Could not parse $mpResultPath : $($_.Exception.Message)"
                    }
                }
                # $legCode is $null only if the leg never produced an exit code at
                # all, which would make 'did it pass?' unanswerable. Passing $null
                # to an [int] parameter silently becomes 0, i.e. green, so convert
                # it to an unmistakable failure instead.
                $legExitCode = if ($null -eq $legCode) { -1 } else { [int]$legCode }
                $verdict = Get-EngineLegVerdict -Summary $summary -OrchestratorResult $mpResult `
                    -RequireOrchestrator -RequireDoctest:$first -OrchestratorExitCode $legExitCode
                $legReasons = @($verdict.Reasons)
                if ($summaryError) { $legReasons += $summaryError }

                $legs += [pscustomobject]@{
                    version     = $engine.Version
                    status      = $verdict.Status
                    tests       = $verdict.Tests
                    passing     = $verdict.Passing
                    failing     = $verdict.Failing
                    pending     = $verdict.Pending
                    mp_total    = $verdict.MpTotal
                    mp_passed   = $verdict.MpPassed
                    mp_failed   = $verdict.MpFailed
                    mp_skipped  = $verdict.MpSkipped
                    duration_ms = $verdict.DurationMs
                    exit_code   = $legCode
                    reasons     = @($legReasons)
                    gaps        = @($verdict.Gaps)
                    summary     = $summaryPath
                    log         = $legLog
                }
                Write-Host "    Godot $($engine.Version): $(Get-StatusLabel $verdict.Status) (orchestrator exit $legCode)" -ForegroundColor Yellow
                $first = $false
            }
        } finally {
            Write-Phase "Restoring Xbox sandbox to $originalSandbox"
            try {
                $now = Get-CurrentSandbox -ExePath $sandboxExe
                if ($now -ine $originalSandbox) {
                    Set-CurrentSandbox -ExePath $sandboxExe -SandboxId $originalSandbox -LogPath $sandboxLog | Out-Null
                }
                Write-Note "Sandbox restored to $originalSandbox"
                Remove-Item -LiteralPath $restoreNote -ErrorAction SilentlyContinue
            } catch {
                Write-Error "FAILED to restore the Xbox sandbox. Restore it manually: & `"$sandboxExe`" $originalSandbox /noApps  ($_)"
            }
        }

        # --- Release build -----------------------------------------------------
        Write-Phase 'Building Release'
        $releaseLog = Join-Path $logsDir 'build-release.log'
        $releaseStatus = 'pass'
        $code = Invoke-Logged -FilePath 'cmake' -ArgumentList @('--preset', 'default-release') -LogPath $releaseLog -WorkingDirectory $checkoutDir -ScrubCredentials
        if ($code -eq 0) {
            $code = Invoke-Logged -FilePath 'cmake' -ArgumentList @('--build', '--preset', 'release') -LogPath $releaseLog -WorkingDirectory $checkoutDir -ScrubCredentials
        }
        if ($code -ne 0) { $releaseStatus = 'fail' }
        Write-Note "Release build: $releaseStatus"

        # --- Verdict and report ------------------------------------------------
        # Re-read the head *before* computing the verdict: if the PR moved while
        # this run was in progress, the evidence describes an obsolete commit and
        # must never be published under a PASS heading.
        $currentHead = (Get-PullRequestMetadata -Number $PullRequest).headRefOid
        $headMoved = ($currentHead -ne $headSha)

        $statuses = @($legs | ForEach-Object { $_.status })
        if ($releaseStatus -ne 'pass') { $statuses += 'fail' }
        # A narrowed matrix is honest coverage, not a failure, but it must never
        # settle as an unqualified pass: the report's claim is full-matrix.
        if ($matrix.Narrowed) { $statuses += 'incomplete' }
        # Likewise for an SDK the candidate does not declare as its default: the
        # run is real evidence, just not evidence for the version under review.
        if ($gdkIdentityGap) { $statuses += 'incomplete' }
        if ($headMoved) { $statuses += 'incomplete' }
        $runStatus = Get-WorstStatus $statuses

        $finishedAt = [datetime]::UtcNow
        $runReasons = @()
        $runGaps = @()
        if ($releaseStatus -ne 'pass') { $runReasons += 'The Release build failed; see build-release.log.' }
        if ($matrix.Narrowed) {
            $runGaps += "Godot matrix narrowed by request: ran $($matrix.Selected -join ', '); did not run $($matrix.Skipped -join ', ')."
        }
        if ($gdkIdentityGap) { $runGaps += $gdkIdentityGap }
        if ($gdkCoverageGap) { $runGaps += $gdkCoverageGap }
        if ($headMoved) {
            $runGaps += "The pull request head moved to ``$currentHead`` while this run was in progress; these results describe ``$headSha`` only and are not validation of the current head."
        }

        $manifest = [pscustomobject]@{
            schema_version       = 1
            pull_request         = $PullRequest
            repository           = $script:Repository
            head_sha             = $headSha
            head_sha_at_finish   = $currentHead
            status               = $runStatus
            started_at           = $startedAt.ToString('o')
            finished_at          = $finishedAt.ToString('o')
            gdk_edition          = $sdk.Edition
            ms_gdk_version       = $sdk.Package
            gdk_pinned           = [bool]$gdk.Pinned
            gdk_declared_default = $gdkManifest.Default
            gdk_not_covered      = @($gdk.Uncovered)
            playfab_title_id     = $script:PlayFabTitleId
            sandbox_id           = $script:SandboxId
            sandbox_restored_to  = $originalSandbox
            release_build_status = $releaseStatus
            run_directory        = $runDir
            engines              = @($legs)
            engines_skipped      = @($matrix.Skipped)
            reasons              = @($runReasons)
            gaps                 = @($runGaps)
        }

        $manifestOut = Join-Path $runDir 'validation-manifest.json'
        $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestOut -Encoding utf8

        $comment = Format-ValidationComment -Manifest $manifest
        $reportOut = Join-Path $runDir 'report.md'
        Set-Content -LiteralPath $reportOut -Value $comment -Encoding utf8

        Write-Phase "Result: $(Get-StatusLabel $runStatus)"
        Write-Host $comment

        if ($NoComment) {
            Write-Note "-NoComment was passed; the report was not posted. It is at $reportOut"
        } else {
            $code = Invoke-Logged -FilePath 'gh' `
                -ArgumentList @('pr', 'comment', "$PullRequest", '--repo', $script:Repository, '--body-file', $reportOut) `
                -LogPath (Join-Path $logsDir 'gh-comment.log')
            if ($code -ne 0) {
                Write-Error "Posting the PR comment failed (exit $code). The report is at $reportOut"
            } else {
                Write-Note "Posted validation comment to $($pr.url)"
            }
        }

        $exitCode = Get-ExitCodeForStatus $runStatus
    } finally {
        $lock.ReleaseMutex()
        $lock.Dispose()
    }
} catch {
    Write-Error $_
    $runStatus = 'error'
    $exitCode = 1
}

exit $exitCode
