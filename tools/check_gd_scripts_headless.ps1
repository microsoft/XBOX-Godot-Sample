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
            ForEach-Object { [System.IO.Path]::GetFullPath((Join-Path $script:RepoRoot $_)) }
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

    Copy-Item -Path (Join-Path $script:RepoRoot 'addons') -Destination $tempRoot -Recurse -Force
    return $tempRoot
}

function New-ProjectValidationRoot {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourceProjectRoot
    )

    $tempRoot = Join-Path ([System.IO.Path]::GetTempPath()) ("godot-project-hook-" + [System.Guid]::NewGuid().ToString('N'))
    New-Item -Path $tempRoot -ItemType Directory | Out-Null
    $script:TempPaths.Add($tempRoot)

    Get-ChildItem -Path $SourceProjectRoot -Force |
        Where-Object { $_.Name -notin @('.godot', 'project.godot') } |
        Copy-Item -Destination $tempRoot -Recurse -Force

    @'
; Engine configuration file.
; Temporary project used by the git hook.
config_version=5

[application]
config/name="GDScriptHookValidation"
'@ | Set-Content -Path (Join-Path $tempRoot 'project.godot') -Encoding ASCII

    return $tempRoot
}

function Get-ValidationContext {
    param(
        [Parameter(Mandatory = $true)]
        [string]$FilePath,
        [Parameter(Mandatory = $true)]
        [string[]]$ProjectRoots,
        [Parameter(Mandatory = $true)]
        [hashtable]$ProjectValidationRoots,
        [Parameter(Mandatory = $true)]
        [ref]$AddonContext
    )

    $projectRoot = Get-ContainingProjectRoot -FilePath $FilePath -ProjectRoots $ProjectRoots
    if ($null -ne $projectRoot) {
        if (-not $ProjectValidationRoots.ContainsKey($projectRoot)) {
            $ProjectValidationRoots[$projectRoot] = New-ProjectValidationRoot -SourceProjectRoot $projectRoot
        }

        $relativePath = Get-RelativePath -BasePath $projectRoot -TargetPath $FilePath
        return [pscustomobject]@{
            Key         = $projectRoot
            DisplayName = Get-RepoRelativePath -Path $projectRoot
            ProjectRoot = $ProjectValidationRoots[$projectRoot]
            RealRoot    = $projectRoot
            ScriptPath  = 'res://' + ($relativePath -replace '\\', '/')
        }
    }

    $addonsRoot = Join-Path $script:RepoRoot 'addons'
    if ($FilePath.StartsWith($addonsRoot + [System.IO.Path]::DirectorySeparatorChar, [System.StringComparison]::OrdinalIgnoreCase)) {
        if ($null -eq $AddonContext.Value) {
            $AddonContext.Value = [pscustomobject]@{
                Key         = 'addons'
                DisplayName = 'addons'
                ProjectRoot = New-AddonValidationProject
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
    $projectValidationRoots = @{}
    $addonContext = $null

    foreach ($filePath in $gdFiles) {
        $context = Get-ValidationContext -FilePath $filePath -ProjectRoots $projectRoots -ProjectValidationRoots $projectValidationRoots -AddonContext ([ref]$addonContext)
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
