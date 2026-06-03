<#
.SYNOPSIS
    Validates all GDScript (.gd) files in the repository using Godot's headless
    parser.

.DESCRIPTION
    This script enumerates every .gd file in the current git working tree
    (tracked plus untracked, non-ignored files), groups them by Godot project
    context, and runs `godot --headless --check-only --script <path>` against
    each one. It reports parse errors and warnings, then exits non-zero if any
    script fails validation.

    Scripts that live inside a Godot project (a directory containing
    project.godot) are checked in-place against that project root.  Scripts
    under the repo-root addons/ directory — which has no project.godot of its
    own — are checked against a lightweight temporary project so that Godot can
    resolve res:// paths for them.

    Godot's --check-only flag only parses the specified script (plus any
    transitive preload/extends dependencies).  It does NOT execute autoloads,
    load editor plugins, or modify the .godot/ cache, so checking against the
    real project root is safe and avoids copying entire project trees.

.NOTES
    Requires a Godot 4.x console executable.  The script searches for one in
    the following order:
      1. GODOT_CONSOLE / GODOT_BIN / GODOT environment variables
      2. Godot*_console.exe under sample/
      3. Godot*.exe under sample/
      4. 'godot' or 'godot4' on PATH
#>
[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$script:RepoRoot = [System.IO.Path]::GetFullPath((& git rev-parse --show-toplevel).Trim())
$script:TempPaths = [System.Collections.Generic.List[string]]::new()

function Get-GodotExecutable {
    $candidates = [System.Collections.Generic.List[string]]::new()

    foreach ($envName in @('GODOT_CONSOLE', 'GODOT_BIN', 'GODOT')) {
        $value = [Environment]::GetEnvironmentVariable($envName)
        if (-not [string]::IsNullOrWhiteSpace($value)) {
            $candidates.Add($value)
        }
    }

    $repoGodotCandidates = Get-ChildItem -Path (Join-Path $script:RepoRoot 'sample') -Filter 'Godot*_console.exe' -File -ErrorAction SilentlyContinue |
        Sort-Object FullName -Descending
    foreach ($candidate in $repoGodotCandidates) {
        $candidates.Add($candidate.FullName)
    }

    $repoFallbackCandidates = Get-ChildItem -Path (Join-Path $script:RepoRoot 'sample') -Filter 'Godot*.exe' -File -ErrorAction SilentlyContinue |
        Sort-Object FullName -Descending
    foreach ($candidate in $repoFallbackCandidates) {
        $candidates.Add($candidate.FullName)
    }

    foreach ($commandName in @('godot', 'godot4')) {
        $command = Get-Command $commandName -ErrorAction SilentlyContinue
        if ($null -ne $command -and -not [string]::IsNullOrWhiteSpace($command.Source)) {
            $candidates.Add($command.Source)
        }
    }

    foreach ($candidate in ($candidates | Select-Object -Unique)) {
        if (Test-Path $candidate) {
            return [System.IO.Path]::GetFullPath((Resolve-Path $candidate).Path)
        }
    }

    throw "Could not find a Godot executable. Set GODOT_CONSOLE or GODOT_BIN, or place a Godot console executable under sample\."
}

function Get-GDScriptFiles {
    $files = & git -C $script:RepoRoot ls-files --cached --others --exclude-standard -- '*.gd'
    if ($LASTEXITCODE -ne 0) {
        throw 'Failed to enumerate .gd files from git.'
    }

    return @(
        $files |
            Where-Object { -not [string]::IsNullOrWhiteSpace($_) } |
            Sort-Object -Unique |
            ForEach-Object { [System.IO.Path]::GetFullPath((Join-Path $script:RepoRoot $_)) } |
            Where-Object { Test-Path -LiteralPath $_ -PathType Leaf }
    )
}

function Get-ProjectRoots {
    return @(
        Get-ChildItem -Path $script:RepoRoot -Recurse -Filter 'project.godot' -File |
            ForEach-Object { $_.Directory.FullName } |
            Sort-Object Length -Descending
    )
}

function Get-RelativePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$BasePath,
        [Parameter(Mandatory = $true)]
        [string]$TargetPath
    )

    $normalizedBase = [System.IO.Path]::GetFullPath($BasePath).TrimEnd('\') + '\'
    $normalizedTarget = [System.IO.Path]::GetFullPath($TargetPath)
    $baseUri = [System.Uri]$normalizedBase
    $targetUri = [System.Uri]$normalizedTarget
    return [System.Uri]::UnescapeDataString($baseUri.MakeRelativeUri($targetUri).ToString()) -replace '/', '\'
}

function Get-RepoRelativePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    return Get-RelativePath -BasePath $script:RepoRoot -TargetPath $Path
}

function Get-ContainingProjectRoot {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [Parameter(Mandatory = $true)]
        [string[]]$ProjectRoots
    )

    foreach ($projectRoot in $ProjectRoots) {
        if ($FilePath.StartsWith($projectRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase) -or
            $FilePath.Equals($projectRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
            return $projectRoot
        }
    }

    return $null
}

function New-AddonValidationProject {
    param(
        [Parameter(Mandatory = $true)]
        [string]$AddonSourceDir
    )

    # Standalone addon directories (e.g. addons/, sample/addons/) have no
    # project.godot, so Godot needs a temporary project root that contains
    # them.  Only the addon tree is copied — no full project duplication.
    $tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("godot-gd-hook-" + [System.Guid]::NewGuid().ToString('N'))
    New-Item -Path $tempRoot -ItemType Directory | Out-Null
    $script:TempPaths.Add($tempRoot)

    @'
; Engine configuration file.
; Temporary project used by the git hook.
config_version=5

[application]
config/name="GDScriptHookValidation"
'@ | Set-Content -Path (Join-Path $tempRoot 'project.godot') -Encoding ASCII

    Copy-Item -Path $AddonSourceDir -Destination $tempRoot -Recurse -Force
    return $tempRoot
}

function Get-ValidationContext {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [Parameter(Mandatory = $true)]
        [string[]]$ProjectRoots,
        [Parameter(Mandatory = $true)]
        [ref]$AddonContext,
        [Parameter(Mandatory = $true)]
        [ref]$SampleAddonContext
    )

    # 1. Script lives inside a Godot project — check in-place.
    $projectRoot = Get-ContainingProjectRoot -FilePath $FilePath -ProjectRoots $ProjectRoots
    if ($null -ne $projectRoot) {
        $relativePath = Get-RelativePath -BasePath $projectRoot -TargetPath $FilePath
        return [pscustomobject]@{
            Key         = $projectRoot
            DisplayName = Get-RepoRelativePath -Path $projectRoot
            ProjectRoot = $projectRoot
            RealRoot    = $projectRoot
            ScriptPath  = 'res://' + ($relativePath -replace '\\', '/')
        }
    }

    # 2. Script lives under repo-root addons/ (no project.godot) — use a
    #    lightweight temp project so Godot can resolve res:// paths.
    $addonsRoot = Join-Path $script:RepoRoot 'addons'
    if ($FilePath.StartsWith($addonsRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        if ($null -eq $AddonContext.Value) {
            $AddonContext.Value = [pscustomobject]@{
                Key         = 'addons'
                DisplayName = 'addons'
                ProjectRoot = New-AddonValidationProject -AddonSourceDir $addonsRoot
                RealRoot    = $script:RepoRoot
            }
        }

        $relativePath = Get-RelativePath -BasePath $script:RepoRoot -TargetPath $FilePath
        return [pscustomobject]@{
            Key         = $AddonContext.Value.Key
            DisplayName = $AddonContext.Value.DisplayName
            ProjectRoot = $AddonContext.Value.ProjectRoot
            RealRoot    = $AddonContext.Value.RealRoot
            ScriptPath  = 'res://' + ($relativePath -replace '\\', '/')
        }
    }

    # 3. Script lives under sample/addons/ (shared addon source, no
    #    project.godot) — same temp-project approach.
    $sampleAddonsRoot = Join-Path $script:RepoRoot 'sample' 'addons'
    if ($FilePath.StartsWith($sampleAddonsRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        if ($null -eq $SampleAddonContext.Value) {
            $SampleAddonContext.Value = [pscustomobject]@{
                Key         = 'sample-addons'
                DisplayName = 'sample\addons'
                ProjectRoot = New-AddonValidationProject -AddonSourceDir $sampleAddonsRoot
                RealRoot    = Join-Path $script:RepoRoot 'sample'
            }
        }

        $sampleRoot = Join-Path $script:RepoRoot 'sample'
        $relativePath = Get-RelativePath -BasePath $sampleRoot -TargetPath $FilePath
        return [pscustomobject]@{
            Key         = $SampleAddonContext.Value.Key
            DisplayName = $SampleAddonContext.Value.DisplayName
            ProjectRoot = $SampleAddonContext.Value.ProjectRoot
            RealRoot    = $SampleAddonContext.Value.RealRoot
            ScriptPath  = 'res://' + ($relativePath -replace '\\', '/')
        }
    }

    throw "No Godot project context was found for '$FilePath'."
}

function Convert-GodotOutput {
    param(
        [Parameter(Mandatory = $true)]
        [object[]]$Output,
        [Parameter(Mandatory = $true)]
        [string]$RealRoot
    )

    $realRootFull = [System.IO.Path]::GetFullPath($RealRoot)
    $normalizedLines = [System.Collections.Generic.List[string]]::new()

    foreach ($entry in $Output) {
        if ($null -eq $entry) {
            continue
        }

        $line = $entry.ToString() -replace '[\x00-\x08\x0B\x0C\x0E-\x1F]', ''
        $line = $line.TrimEnd()
        if ([string]::IsNullOrWhiteSpace($line)) {
            continue
        }

        if ($line -match '^(Godot Engine \(Console\))?(Godot Engine v.+https://godotengine\.org)?$') {
            continue
        }
        if ($line -match '^(Godot Engine v.+https://godotengine\.org)$') {
            continue
        }
        if ($line -match '^(Project metadata update required|Using exit code)$') {
            continue
        }
        if ($line -match 'pwsh\.exe$' -or $line -match 'powershell\.exe$') {
            continue
        }

        $line = [regex]::Replace($line, 'res://[A-Za-z0-9_./-]+', {
            param($match)
            $relativePath = $match.Value.Substring(6) -replace '/', '\'
            return [System.IO.Path]::GetFullPath((Join-Path $realRootFull $relativePath))
        })

        $normalizedLines.Add($line)
    }

    return @($normalizedLines.ToArray())
}

function Test-GodotIssuesPresent {
    param(
        [AllowEmptyCollection()]
        [string[]]$Lines
    )

    foreach ($line in $Lines) {
        if ($line -match '(^| )(SCRIPT ERROR:|ERROR:|WARNING:)') {
            return $true
        }
    }

    return $false
}

function Invoke-GodotCheckOnly {
    param(
        [Parameter(Mandatory = $true)]
        [string]$GodotExecutable,
        [Parameter(Mandatory = $true)]
        [string]$ProjectRoot,
        [Parameter(Mandatory = $true)]
        [string]$ScriptPath
    )

    $stdoutPath = [System.IO.Path]::GetTempFileName()
    $stderrPath = [System.IO.Path]::GetTempFileName()

    try {
        $process = Start-Process -FilePath $GodotExecutable `
            -ArgumentList @('--headless', '--path', $ProjectRoot, '--check-only', '--script', $ScriptPath, '--', '--gd-script-check') `
            -RedirectStandardOutput $stdoutPath `
            -RedirectStandardError $stderrPath `
            -NoNewWindow `
            -Wait `
            -PassThru

        $output = @()
        if (Test-Path $stdoutPath) {
            $output += Get-Content -Path $stdoutPath
        }
        if (Test-Path $stderrPath) {
            $output += Get-Content -Path $stderrPath
        }

        return [pscustomobject]@{
            ExitCode = $process.ExitCode
            Output   = @($output)
        }
    }
    finally {
        Remove-Item -Path $stdoutPath, $stderrPath -Force -ErrorAction SilentlyContinue
    }
}

function Invoke-GodotScriptCheck {
    param(
        [Parameter(Mandatory = $true)]
        [string]$GodotExecutable,
        [Parameter(Mandatory = $true)]
        [pscustomobject]$Context
    )

    $scriptPaths = @($Context.ScriptPaths | Sort-Object -Unique)
    if ($scriptPaths.Count -eq 0) {
        return @()
    }

    $suffix = if ($scriptPaths.Count -eq 1) { '' } else { 's' }
    Write-Host ("Checking {0} ({1} script{2})" -f $Context.DisplayName, $scriptPaths.Count, $suffix)

    $failures = [System.Collections.Generic.List[object]]::new()
    foreach ($scriptPath in $scriptPaths) {
        $result = Invoke-GodotCheckOnly -GodotExecutable $GodotExecutable -ProjectRoot $Context.ProjectRoot -ScriptPath $scriptPath
        $normalized = Convert-GodotOutput -Output $result.Output -RealRoot $Context.RealRoot
        if ($null -eq $normalized) {
            $normalized = @()
        }

        if ($result.ExitCode -ne 0 -or (Test-GodotIssuesPresent -Lines $normalized)) {
            $scriptRelativePath = $scriptPath.Substring(6) -replace '/', '\'
            $failures.Add([pscustomobject]@{
                ContextDisplayName = $Context.DisplayName
                ScriptPath         = [System.IO.Path]::GetFullPath((Join-Path $Context.RealRoot $scriptRelativePath))
                Output             = $normalized
            })
        }
    }

    return @($failures)
}

try {
    $godotExecutable = Get-GodotExecutable
    $gdFiles = Get-GDScriptFiles
    if ($gdFiles.Count -eq 0) {
        Write-Host 'No .gd files found; skipping headless GDScript validation.'
        exit 0
    }

    $projectRoots = Get-ProjectRoots
    $contexts = @{}
    $addonContext = $null
    $sampleAddonContext = $null

    foreach ($filePath in $gdFiles) {
        $context = Get-ValidationContext -FilePath $filePath -ProjectRoots $projectRoots -AddonContext ([ref]$addonContext) -SampleAddonContext ([ref]$sampleAddonContext)
        if (-not $contexts.ContainsKey($context.Key)) {
            $contexts[$context.Key] = [pscustomobject]@{
                DisplayName = $context.DisplayName
                ProjectRoot = $context.ProjectRoot
                RealRoot    = $context.RealRoot
                ScriptPaths = [System.Collections.Generic.List[string]]::new()
            }
        }

        $contexts[$context.Key].ScriptPaths.Add($context.ScriptPath)
    }

    $allFailures = [System.Collections.Generic.List[object]]::new()
    foreach ($context in ($contexts.Values | Sort-Object DisplayName)) {
        $failures = Invoke-GodotScriptCheck -GodotExecutable $godotExecutable -Context $context
        foreach ($failure in $failures) {
            $allFailures.Add($failure)
        }
    }

    if ($allFailures.Count -gt 0) {
        Write-Host ''
        Write-Host 'Headless GDScript validation failed.' -ForegroundColor Red
        foreach ($failure in $allFailures) {
            Write-Host ''
            Write-Host ("[{0}] {1}" -f $failure.ContextDisplayName, $failure.ScriptPath) -ForegroundColor Yellow
            foreach ($line in $failure.Output) {
                Write-Host $line
            }
        }
        exit 1
    }

    Write-Host 'Headless GDScript validation passed.'
    exit 0
}
finally {
    foreach ($path in $script:TempPaths) {
        if (Test-Path $path) {
            Remove-Item -Path $path -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}
