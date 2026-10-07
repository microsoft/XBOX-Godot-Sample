<#
.SYNOPSIS
    Shared acquisition of pinned stock Godot builds.

.DESCRIPTION
    Single implementation of the supply-chain policy that was previously inline
    in .github\actions\setup-godot\action.yml:

      1. The version must appear in .github\godot-versions.json's 'sha512' map.
         An unpinned version is refused -- never downloaded "just to see".
      2. The archive is downloaded into a staging directory OUTSIDE the final
         destination, so a tampered or truncated download can never poison a
         cache or an engine directory.
      3. SHA-512 is verified BEFORE extraction.
      4. Only official godotengine/godot-builds release assets are fetched.

    Both consumers use this module:
      - .github\actions\setup-godot (CI, via tools\ci\get_godot.ps1)
      - tools\validate_pr_local.ps1 (local PR validation, run-private engines)

    Everything here is offline-testable: the network and filesystem boundaries
    are injectable (-Downloader), so tools\ci\tests\pr_local_validation.test.ps1
    exercises the refusal paths without touching the network.
#>

Set-StrictMode -Version Latest

# A godot-builds release tag: 4.7.1-stable, 4.5-stable, 4.6.1-rc1, ...
# Deliberately strict: the version is interpolated into a URL and a filesystem
# path, so '..', separators, and shell metacharacters must never get through.
$script:GodotVersionRegex = '^[0-9]+\.[0-9]+(\.[0-9]+)?-[A-Za-z0-9]+$'
$script:Sha512Regex = '^[0-9a-f]{128}$'
$script:GodotBuildsBaseUrl = 'https://github.com/godotengine/godot-builds/releases/download'

function Test-GodotVersionString {
    <#
    .SYNOPSIS
        True when $Version is a syntactically valid godot-builds release tag.
    #>
    param([Parameter(Mandatory = $true)][AllowEmptyString()][string]$Version)
    return [bool]([regex]::IsMatch($Version, $script:GodotVersionRegex))
}

function Get-GodotAssetName {
    param(
        [Parameter(Mandatory = $true)][string]$Version,
        [ValidateSet('console', 'editor')][string]$Flavor = 'console'
    )
    if (-not (Test-GodotVersionString $Version)) {
        throw "Invalid Godot version string '$Version'. Expected a godotengine/godot-builds release tag such as '4.7.1-stable'."
    }
    # Both flavors ship inside the same zip; the zip name never varies by flavor.
    return "Godot_v${Version}_win64.exe.zip"
}

function Get-GodotExecutableName {
    param(
        [Parameter(Mandatory = $true)][string]$Version,
        [ValidateSet('console', 'editor')][string]$Flavor = 'console'
    )
    if (-not (Test-GodotVersionString $Version)) {
        throw "Invalid Godot version string '$Version'."
    }
    if ($Flavor -eq 'editor') { return "Godot_v${Version}_win64.exe" }
    return "Godot_v${Version}_win64_console.exe"
}

function Read-GodotManifest {
    <#
    .SYNOPSIS
        Parse .github\godot-versions.json into default/supported/sha512.
    #>
    param([Parameter(Mandatory = $true)][string]$ManifestPath)

    if (-not (Test-Path -LiteralPath $ManifestPath)) {
        throw "Godot versions manifest not found at '$ManifestPath'."
    }
    try {
        $manifest = Get-Content -LiteralPath $ManifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
    } catch {
        throw "Godot versions manifest '$ManifestPath' is not valid JSON: $($_.Exception.Message)"
    }

    $names = @($manifest.PSObject.Properties.Name)
    foreach ($required in @('default', 'supported', 'sha512')) {
        if ($names -notcontains $required) {
            throw "Godot versions manifest '$ManifestPath' is missing the required '$required' field."
        }
    }

    $supported = @($manifest.supported)
    if ($supported.Count -eq 0) {
        throw "Godot versions manifest '$ManifestPath' lists no supported versions."
    }
    foreach ($v in $supported) {
        if (-not (Test-GodotVersionString ([string]$v))) {
            throw "Godot versions manifest '$ManifestPath' lists an invalid supported version '$v'."
        }
    }
    if (-not (Test-GodotVersionString ([string]$manifest.default))) {
        throw "Godot versions manifest '$ManifestPath' has an invalid 'default' version '$($manifest.default)'."
    }
    if ($supported -notcontains [string]$manifest.default) {
        throw "Godot versions manifest '$ManifestPath' default '$($manifest.default)' is not in its own 'supported' list."
    }

    return [pscustomobject]@{
        Path      = [string]$ManifestPath
        Default   = [string]$manifest.default
        Supported = @($supported | ForEach-Object { [string]$_ })
        Sha512    = $manifest.sha512
    }
}

function Get-GodotPin {
    <#
    .SYNOPSIS
        Resolve the pinned SHA-512 + official asset URL for one version.
    .DESCRIPTION
        Refuses an unpinned or malformed hash. This runs on every acquisition,
        including cache hits, so an unpinned version is rejected up front and
        the hash can be folded into a cache key.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Version,
        [Parameter(Mandatory = $true)][string]$ManifestPath,
        [ValidateSet('console', 'editor')][string]$Flavor = 'console'
    )

    if (-not (Test-GodotVersionString $Version)) {
        throw "Invalid Godot version string '$Version'. Expected a godotengine/godot-builds release tag such as '4.7.1-stable'."
    }

    $manifest = Read-GodotManifest -ManifestPath $ManifestPath

    $expected = $null
    if ($null -ne $manifest.Sha512 -and @($manifest.Sha512.PSObject.Properties.Name) -contains $Version) {
        $expected = [string]$manifest.Sha512.$Version
    }
    if ([string]::IsNullOrWhiteSpace($expected)) {
        throw "No pinned SHA-512 for Godot '$Version' in '$ManifestPath' ('sha512' map). Refusing to use an unverified binary -- add the hash of $(Get-GodotAssetName -Version $Version) to harden the supply chain."
    }
    $expected = $expected.Trim().ToLowerInvariant()
    if (-not [regex]::IsMatch($expected, $script:Sha512Regex)) {
        throw "Pinned hash for Godot '$Version' in '$ManifestPath' is not a 128-character lowercase hex SHA-512: '$expected'."
    }

    $asset = Get-GodotAssetName -Version $Version -Flavor $Flavor
    return [pscustomobject]@{
        Version        = $Version
        Flavor         = $Flavor
        Sha512         = $expected
        Asset          = $asset
        Url            = "$script:GodotBuildsBaseUrl/$Version/$asset"
        ExecutableName = Get-GodotExecutableName -Version $Version -Flavor $Flavor
    }
}

function Assert-FileSha512 {
    <#
    .SYNOPSIS
        Throw unless $Path hashes to $Expected. Returns the actual hash.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Path,
        [Parameter(Mandatory = $true)][string]$Expected
    )
    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Cannot verify SHA-512: '$Path' does not exist."
    }
    $actual = (Get-FileHash -Algorithm SHA512 -LiteralPath $Path).Hash.ToLowerInvariant()
    if ($actual -ne $Expected.ToLowerInvariant()) {
        throw "SHA-512 mismatch for '$([System.IO.Path]::GetFileName($Path))'`n  expected: $Expected`n  actual:   $actual`nAborting before extraction (possible tampering or a stale pin)."
    }
    return $actual
}

function ConvertTo-GodotVersionPrefix {
    <#
    .SYNOPSIS
        '4.7.1-stable' -> '4.7.1.stable' (the shape `--version` reports).
    #>
    param([Parameter(Mandatory = $true)][string]$Version)
    return ($Version -replace '-', '.')
}

function Test-GodotReportedVersion {
    <#
    .SYNOPSIS
        True when `--version` output identifies the expected release tag.
    .DESCRIPTION
        Godot prints e.g. '4.7.1.stable.official.abcdef123'. A private engine
        directory that silently holds a different build must not be accepted:
        the wrapper reports a per-engine result, so a mislabeled engine would
        misattribute coverage.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Version,
        [Parameter(Mandatory = $true)][AllowEmptyString()][AllowNull()][string]$Reported
    )
    if ([string]::IsNullOrWhiteSpace($Reported)) { return $false }
    $prefix = ConvertTo-GodotVersionPrefix -Version $Version
    foreach ($line in ($Reported -split "`r?`n")) {
        $t = $line.Trim()
        if ($t.Length -eq 0) { continue }
        if ($t -eq $prefix -or $t.StartsWith("$prefix.")) { return $true }
    }
    return $false
}

function Resolve-GodotExecutable {
    <#
    .SYNOPSIS
        Find the flavored executable inside an extracted engine directory.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Root,
        [Parameter(Mandatory = $true)][string]$Version,
        [ValidateSet('console', 'editor')][string]$Flavor = 'console'
    )
    $pattern = Get-GodotExecutableName -Version $Version -Flavor $Flavor
    if (-not (Test-Path -LiteralPath $Root)) { return $null }
    $exe = Get-ChildItem -LiteralPath $Root -Recurse -Filter $pattern -File -ErrorAction SilentlyContinue |
        Select-Object -First 1
    if (-not $exe) { return $null }
    return $exe.FullName
}

function Invoke-GodotDefaultDownload {
    param(
        [Parameter(Mandatory = $true)][string]$Url,
        [Parameter(Mandatory = $true)][string]$OutFile
    )
    $previous = $ProgressPreference
    $ProgressPreference = 'SilentlyContinue'
    try {
        Invoke-WebRequest -Uri $Url -OutFile $OutFile -MaximumRedirection 5 -UseBasicParsing
    } finally {
        $ProgressPreference = $previous
    }
}

function Get-GodotArchive {
    <#
    .SYNOPSIS
        Produce a SHA-512-verified archive for $Pin, using the cache if valid.
    .DESCRIPTION
        Only verified archives are cached. A cached archive is re-verified on
        every use -- a cache is a speed optimization, never a trust anchor.
    #>
    param(
        [Parameter(Mandatory = $true)][psobject]$Pin,
        [Parameter(Mandatory = $true)][string]$StagingDir,
        [string]$ArchiveCacheDir,
        [scriptblock]$Downloader
    )

    if (-not $Downloader) { $Downloader = ${function:Invoke-GodotDefaultDownload} }

    if ($ArchiveCacheDir) {
        $cached = Join-Path $ArchiveCacheDir $Pin.Asset
        if (Test-Path -LiteralPath $cached) {
            try {
                [void](Assert-FileSha512 -Path $cached -Expected $Pin.Sha512)
                return [pscustomobject]@{ Path = $cached; FromCache = $true; Url = $Pin.Url }
            } catch {
                # A stale pin or a corrupt cache entry: drop it and re-download.
                Write-Verbose "Discarding unverifiable cached archive '$cached': $($_.Exception.Message)"
                Remove-Item -LiteralPath $cached -Force -ErrorAction SilentlyContinue
            }
        }
    }

    if (Test-Path -LiteralPath $StagingDir) { Remove-Item -LiteralPath $StagingDir -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $StagingDir | Out-Null
    $staged = Join-Path $StagingDir $Pin.Asset

    & $Downloader $Pin.Url $staged
    if (-not (Test-Path -LiteralPath $staged)) {
        throw "Download of '$($Pin.Url)' produced no file at '$staged'."
    }
    [void](Assert-FileSha512 -Path $staged -Expected $Pin.Sha512)

    if ($ArchiveCacheDir) {
        New-Item -ItemType Directory -Force -Path $ArchiveCacheDir | Out-Null
        $cached = Join-Path $ArchiveCacheDir $Pin.Asset
        Copy-Item -LiteralPath $staged -Destination $cached -Force
        return [pscustomobject]@{ Path = $cached; FromCache = $false; Url = $Pin.Url }
    }
    return [pscustomobject]@{ Path = $staged; FromCache = $false; Url = $Pin.Url }
}

function Install-Godot {
    <#
    .SYNOPSIS
        Ensure a verified Godot of $Version is extracted under $DestinationRoot.
    .DESCRIPTION
        Extracts into <DestinationRoot>\<Version>. Callers that need isolation
        (the local PR validator stages per-run redist DLLs beside the engine)
        pass a run-private DestinationRoot so no other run's DLLs are on the
        process search path.
    .OUTPUTS
        PSCustomObject: Version, Flavor, Path, Sha512, EngineDir, Url,
        FromCache, Downloaded, ReportedVersion.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Version,
        [Parameter(Mandatory = $true)][string]$ManifestPath,
        [Parameter(Mandatory = $true)][string]$DestinationRoot,
        [ValidateSet('console', 'editor')][string]$Flavor = 'console',
        [string]$ArchiveCacheDir,
        [string]$StagingRoot,
        [switch]$Force,
        [scriptblock]$Downloader,
        [switch]$SkipVersionProbe
    )

    $pin = Get-GodotPin -Version $Version -ManifestPath $ManifestPath -Flavor $Flavor
    $engineDir = Join-Path $DestinationRoot $Version

    $downloaded = $false
    $fromCache = $false
    $existing = if ($Force) { $null } else { Resolve-GodotExecutable -Root $engineDir -Version $Version -Flavor $Flavor }

    if (-not $existing) {
        if ($Force -and (Test-Path -LiteralPath $engineDir)) {
            Remove-Item -LiteralPath $engineDir -Recurse -Force
        }
        if (-not $StagingRoot) { $StagingRoot = Join-Path $DestinationRoot '.staging' }
        $staging = Join-Path $StagingRoot $Version

        $archive = Get-GodotArchive -Pin $pin -StagingDir $staging -ArchiveCacheDir $ArchiveCacheDir -Downloader $Downloader
        $fromCache = [bool]$archive.FromCache
        $downloaded = -not $fromCache

        New-Item -ItemType Directory -Force -Path $engineDir | Out-Null
        Expand-Archive -LiteralPath $archive.Path -DestinationPath $engineDir -Force
        Remove-Item -LiteralPath $staging -Recurse -Force -ErrorAction SilentlyContinue

        $existing = Resolve-GodotExecutable -Root $engineDir -Version $Version -Flavor $Flavor
        if (-not $existing) {
            throw "Godot executable '$($pin.ExecutableName)' not found under '$engineDir' after extracting $($pin.Asset)."
        }
    }

    $reported = $null
    if (-not $SkipVersionProbe) {
        try {
            $reported = (& $existing --version 2>&1 | Out-String).Trim()
        } catch {
            throw "Failed to run '$existing --version': $($_.Exception.Message)"
        }
        if (-not (Test-GodotReportedVersion -Version $Version -Reported $reported)) {
            throw "Engine at '$existing' reports version '$reported', which does not identify '$Version'. Refusing to attribute results to the wrong engine."
        }
    }

    return [pscustomobject]@{
        Version         = $Version
        Flavor          = $Flavor
        Path            = $existing
        Sha512          = $pin.Sha512
        EngineDir       = $engineDir
        Url             = $pin.Url
        FromCache       = $fromCache
        Downloaded      = $downloaded
        ReportedVersion = $reported
    }
}

Export-ModuleMember -Function @(
    'Test-GodotVersionString',
    'Get-GodotAssetName',
    'Get-GodotExecutableName',
    'Read-GodotManifest',
    'Get-GodotPin',
    'Assert-FileSha512',
    'ConvertTo-GodotVersionPrefix',
    'Test-GodotReportedVersion',
    'Resolve-GodotExecutable',
    'Get-GodotArchive',
    'Install-Godot'
)
