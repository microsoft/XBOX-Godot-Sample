<#
.SYNOPSIS
    Build and zip the GodotGDK addons for drop-in use in a Godot project.

.DESCRIPTION
    Configures the package-specific CMake preset, builds the requested native
    addon configurations, stages only the files a game project should receive,
    then writes a zip whose root contains an `addons\` directory and a
    `GETTING_STARTED.md` quickstart for the recipient.

    The default `Both` configuration includes Debug and Release GDExtension
    DLLs because the addon's .gdextension manifests declare both library paths.
    Use the default zip for editor use plus release exports.

.PARAMETER Configuration
    `Both` (default), `Debug`, or `Release`. `Both` builds and packages both
    native addon DLL variants.

.PARAMETER OutputPath
    Destination zip path. Relative paths are resolved from the repo root.
    Defaults to `build\dist\godot-gdk-addons-debug-release.zip`.

.PARAMETER IncludeDebugSymbols
    Include matching addon PDB files when present. Symbols are omitted by
    default to keep the drop-in zip small.

.PARAMETER Clean
    Remove `build\addon-package` before configuring and building. The staging
    directory is always cleared before packaging.

.PARAMETER Reconfigure
    Force `cmake --preset addon-package` even when the package build directory
    already has a CMake cache.

.OUTPUTS
    Exits 0 on success; otherwise throws with the failing command or missing
    file. The zip can be extracted into a Godot project root.

.EXAMPLE
    .\tools\package_addons.ps1
    Builds Debug and Release addon DLLs and creates the default drop-in zip.

.EXAMPLE
    .\tools\package_addons.ps1 -Configuration Release -OutputPath .\build\dist\godot-gdk-addons-release.zip
    Creates a release-only package.
#>
[CmdletBinding()]
param(
    [ValidateSet('Debug', 'Release', 'Both')]
    [string]$Configuration = 'Both',

    [string]$OutputPath = '',

    [switch]$IncludeDebugSymbols,

    [switch]$Clean,

    [switch]$Reconfigure
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest

$script:RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$script:ConfigurePreset = 'addon-package'
$script:BinaryDirRel = 'build\addon-package'
$script:BinaryDir = Join-Path $script:RepoRoot $script:BinaryDirRel
$script:DistDir = Join-Path $script:RepoRoot 'build\dist'
$script:StageDir = Join-Path $script:DistDir 'godot-gdk-addons'
$script:BuildPresets = @{
    Debug = 'debug-addon-package'
    Release = 'release-addon-package'
}

$script:NativeAddons = @(
    @{ Name = 'godot_gdk';       Manifest = 'godot_gdk.gdextension' },
    @{ Name = 'godot_playfab';   Manifest = 'godot_playfab.gdextension' },
    @{ Name = 'godot_gameinput'; Manifest = 'godot_gameinput.gdextension' }
)

$script:RequiredRuntimeDlls = @{
    godot_gdk = @{
        # Both Thunks variants must be deployed in every config: the Debug
        # addon links Release imports (per CMAKE_MAP_IMPORTED_CONFIG_DEBUG=Release)
        # so it needs Thunks.dll, AND xsapi internals probe for Thunks.Debug.dll
        # at runtime on a Debug addon (empirically required to avoid a
        # deterministic signal-11 shutdown crash). See gdk_xsapi_thunks_dlls()
        # in cmake/GDKDependencies.cmake.
        Debug = @('libHttpClient.dll', 'Microsoft.Xbox.Services.C.Thunks.dll', 'Microsoft.Xbox.Services.C.Thunks.Debug.dll', 'XCurl.dll')
        Release = @('libHttpClient.dll', 'Microsoft.Xbox.Services.C.Thunks.dll', 'Microsoft.Xbox.Services.C.Thunks.Debug.dll', 'XCurl.dll')
    }
    godot_playfab = @{
        Debug = @('libHttpClient.dll', 'Party.dll', 'PlayFabCore.dll', 'PlayFabGameSave.dll', 'PlayFabMultiplayer.dll', 'PlayFabServices.dll')
        Release = @('libHttpClient.dll', 'Party.dll', 'PlayFabCore.dll', 'PlayFabGameSave.dll', 'PlayFabMultiplayer.dll', 'PlayFabServices.dll')
    }
}

function Get-SelectedConfigurations {
    if ($Configuration -eq 'Both') {
        return @('Debug', 'Release')
    }
    return @($Configuration)
}

$script:SelectedConfigurations = @(Get-SelectedConfigurations)

function Invoke-Cmake {
    param([string[]]$Arguments)

    Push-Location $script:RepoRoot
    try {
        & cmake @Arguments
        $exit = $LASTEXITCODE
        if ($exit -ne 0) {
            throw "cmake $($Arguments -join ' ') exited with code $exit"
        }
    } finally {
        Pop-Location
    }
}

function Resolve-OutputPath {
    param([string]$Path)

    if ([string]::IsNullOrWhiteSpace($Path)) {
        $suffix = switch ($Configuration) {
            'Debug' { 'debug' }
            'Release' { 'release' }
            default { 'debug-release' }
        }
        $Path = Join-Path $script:DistDir "godot-gdk-addons-$suffix.zip"
    } elseif (-not [System.IO.Path]::IsPathRooted($Path)) {
        $Path = Join-Path $script:RepoRoot $Path
    }

    return [System.IO.Path]::GetFullPath($Path)
}

function Copy-RequiredFile {
    param(
        [string]$Source,
        [string]$DestinationDirectory
    )

    if (-not (Test-Path -LiteralPath $Source -PathType Leaf)) {
        throw "Required file is missing: $Source"
    }

    New-Item -ItemType Directory -Path $DestinationDirectory -Force | Out-Null
    Copy-Item -LiteralPath $Source -Destination $DestinationDirectory -Force
}

function Copy-DirectoryRequired {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source -PathType Container)) {
        throw "Required directory is missing: $Source"
    }

    New-Item -ItemType Directory -Path $Destination -Force | Out-Null
    Get-ChildItem -LiteralPath $Source -Force |
        Copy-Item -Destination $Destination -Recurse -Force
}

function Test-ConfigurationSelected {
    param([string]$Name)
    return $script:SelectedConfigurations -contains $Name
}

function Should-CopyRuntimeDll {
    param(
        [string]$AddonName,
        [string]$FileName
    )

    $addonDllPattern = '^{0}\.windows\.(debug|release)\.x86_64\.dll$' -f [regex]::Escape($AddonName)
    if ($FileName -match $addonDllPattern) {
        return $false
    }

    # Both XSAPI Thunks DLL variants ship in every config — see the comment
    # on $script:RequiredRuntimeDlls.godot_gdk and gdk_xsapi_thunks_dlls()
    # in cmake/GDKDependencies.cmake. No per-config filtering here.
    return $true
}

function Assert-NativeAddonOutputs {
    param(
        [string]$AddonName,
        [string]$BinDir
    )

    foreach ($config in $script:SelectedConfigurations) {
        $configSuffix = $config.ToLowerInvariant()
        $addonDll = Join-Path $BinDir "$AddonName.windows.$configSuffix.x86_64.dll"
        if (-not (Test-Path -LiteralPath $addonDll -PathType Leaf)) {
            throw "Build did not produce expected $config addon DLL: $addonDll"
        }
    }

    if (-not $script:RequiredRuntimeDlls.ContainsKey($AddonName)) {
        return
    }

    $perConfig = $script:RequiredRuntimeDlls[$AddonName]
    foreach ($config in $script:SelectedConfigurations) {
        if (-not $perConfig.ContainsKey($config)) {
            continue
        }
        foreach ($dllName in @($perConfig[$config])) {
            $runtimeDll = Join-Path $BinDir $dllName
            if (-not (Test-Path -LiteralPath $runtimeDll -PathType Leaf)) {
                throw "Build did not produce expected $AddonName runtime DLL for ${config}: $runtimeDll"
            }
        }
    }
}

function Copy-NativeAddon {
    param(
        [string]$AddonName,
        [string]$Manifest
    )

    $sourceRoot = Join-Path $script:RepoRoot "addons\$AddonName"
    $destRoot = Join-Path $script:StageDir "addons\$AddonName"
    $sourceBin = Join-Path $sourceRoot 'bin'
    $destBin = Join-Path $destRoot 'bin'

    Assert-NativeAddonOutputs -AddonName $AddonName -BinDir $sourceBin

    Copy-RequiredFile -Source (Join-Path $sourceRoot 'plugin.cfg') -DestinationDirectory $destRoot
    Copy-RequiredFile -Source (Join-Path $sourceRoot $Manifest) -DestinationDirectory $destRoot

    foreach ($dirName in @('runtime', 'editor', 'doc_classes')) {
        Copy-DirectoryRequired -Source (Join-Path $sourceRoot $dirName) -Destination (Join-Path $destRoot $dirName)
    }

    New-Item -ItemType Directory -Path $destBin -Force | Out-Null

    foreach ($config in $script:SelectedConfigurations) {
        $configSuffix = $config.ToLowerInvariant()
        Copy-RequiredFile -Source (Join-Path $sourceBin "$AddonName.windows.$configSuffix.x86_64.dll") -DestinationDirectory $destBin

        if ($IncludeDebugSymbols.IsPresent) {
            $pdb = Join-Path $sourceBin "$AddonName.windows.$configSuffix.x86_64.pdb"
            if (Test-Path -LiteralPath $pdb -PathType Leaf) {
                Copy-Item -LiteralPath $pdb -Destination $destBin -Force
            }
        }
    }

    Get-ChildItem -LiteralPath $sourceBin -Filter '*.dll' -File |
        Where-Object { Should-CopyRuntimeDll -AddonName $AddonName -FileName $_.Name } |
        ForEach-Object { Copy-Item -LiteralPath $_.FullName -Destination $destBin -Force }
}

function Copy-PackagingAddon {
    $sourceRoot = Join-Path $script:RepoRoot 'addons\godot_gdk_packaging'
    $destRoot = Join-Path $script:StageDir 'addons\godot_gdk_packaging'

    foreach ($fileName in @('plugin.cfg', 'run.gd', 'gdkpkg.cmd', 'gdkpkg.sh')) {
        Copy-RequiredFile -Source (Join-Path $sourceRoot $fileName) -DestinationDirectory $destRoot
    }

    foreach ($dirName in @('core', 'editor')) {
        Copy-DirectoryRequired -Source (Join-Path $sourceRoot $dirName) -Destination (Join-Path $destRoot $dirName)
    }
}

$outputFullPath = Resolve-OutputPath -Path $OutputPath
$stageFullPath = [System.IO.Path]::GetFullPath($script:StageDir)
$stagePrefix = $stageFullPath.TrimEnd([System.IO.Path]::DirectorySeparatorChar, [System.IO.Path]::AltDirectorySeparatorChar) + [System.IO.Path]::DirectorySeparatorChar
if ($outputFullPath.StartsWith($stagePrefix, [System.StringComparison]::OrdinalIgnoreCase)) {
    throw "OutputPath must not be inside the staging directory: $stageFullPath"
}

Write-Host "package_addons.ps1: Configuration=$Configuration Preset=$($script:ConfigurePreset) Output=$outputFullPath"

if ($Clean.IsPresent -and (Test-Path -LiteralPath $script:BinaryDir)) {
    Write-Host "  Cleaning $($script:BinaryDirRel)"
    Remove-Item -LiteralPath $script:BinaryDir -Recurse -Force
}

$cacheFile = Join-Path $script:BinaryDir 'CMakeCache.txt'
$needConfigure = $Clean.IsPresent -or $Reconfigure.IsPresent -or -not (Test-Path -LiteralPath $cacheFile -PathType Leaf)
if ($needConfigure) {
    Write-Host "  Configuring (cmake --preset $($script:ConfigurePreset))"
    Invoke-Cmake @('--preset', $script:ConfigurePreset)
} else {
    Write-Host "  Reusing existing $($script:BinaryDirRel) (use -Reconfigure to force)"
}

foreach ($config in $script:SelectedConfigurations) {
    $buildPreset = $script:BuildPresets[$config]
    Write-Host "  Building (cmake --build --preset $buildPreset)"
    Invoke-Cmake @('--build', '--preset', $buildPreset)
}

if (Test-Path -LiteralPath $script:StageDir) {
    Write-Host "  Clearing staging directory"
    Remove-Item -LiteralPath $script:StageDir -Recurse -Force
}
New-Item -ItemType Directory -Path (Join-Path $script:StageDir 'addons') -Force | Out-Null

foreach ($addon in $script:NativeAddons) {
    Write-Host "  Staging $($addon.Name)"
    Copy-NativeAddon -AddonName $addon.Name -Manifest $addon.Manifest
}

Write-Host "  Staging godot_gdk_packaging"
Copy-PackagingAddon

Write-Host "  Staging GETTING_STARTED.md"
Copy-RequiredFile `
    -Source (Join-Path $script:RepoRoot 'docs\addon-getting-started.md') `
    -DestinationDirectory $script:StageDir
Rename-Item `
    -LiteralPath (Join-Path $script:StageDir 'addon-getting-started.md') `
    -NewName 'GETTING_STARTED.md'

$outputDirectory = Split-Path -Path $outputFullPath -Parent
New-Item -ItemType Directory -Path $outputDirectory -Force | Out-Null
if (Test-Path -LiteralPath $outputFullPath) {
    Remove-Item -LiteralPath $outputFullPath -Force
}

Write-Host "  Creating zip"
Add-Type -AssemblyName System.IO.Compression.FileSystem
[System.IO.Compression.ZipFile]::CreateFromDirectory(
    $script:StageDir,
    $outputFullPath,
    [System.IO.Compression.CompressionLevel]::Optimal,
    $false)

$stagedFiles = @(Get-ChildItem -LiteralPath $script:StageDir -Recurse -File)
$zipInfo = Get-Item -LiteralPath $outputFullPath
$sizeMiB = [Math]::Round($zipInfo.Length / 1MB, 2)

Write-Host "package_addons.ps1: Created $outputFullPath ($($stagedFiles.Count) files, $sizeMiB MiB)"
