<#
.SYNOPSIS
    Migrate a Godot project from the v0.2.x `GDK*` type names to the v0.3.0 `Xbox*` names.

.DESCRIPTION
    v0.3.0 renamed every script-visible type in the `godot_gdk` addon from the
    `GDK` prefix to the `Xbox` prefix, with no deprecated aliases. This script
    rewrites those references in a consumer project.

    What it rewrites:
      * GDScript / scene / resource / config: `GDKResult` -> `XboxResult`, and the
        other 46 renamed types, plus the `GDKBootstrap` autoload.
      * C#: namespace `GodotGdk` -> `GodotXbox`, static class `Gdk` -> `Xbox`, and
        every `Gdk*` facade type -> `Xbox*`.
      * Packaging forwarder env vars `GDKPKG_*` -> `XBOXPKG_*`.

    What it deliberately leaves alone:
      * A bare `GDK` token in GDScript. The engine singleton is still registered as
        `GDK`, so `GDK.initialize()` must not change. Bare `GDK` used as a *type*
        does need to become `Xbox`, so those occurrences are reported for manual
        review instead of being rewritten. Pass -RewriteBareGdkTypes to rewrite the
        unambiguous type positions automatically.
      * `addons/godot_gdk` paths, `godot_gdk.gdextension`, `gdk_addon_init`,
        `gdk/runtime/*` and `gdk/packaging/*` Project Settings, the `gdk` export
        feature tag, `GodotGdkCSharp` assembly/project names, and
        `cmake/GDKDependencies.cmake`.
      * The bundled addon directories themselves (they already ship renamed).

    All matching is case-sensitive, so lowercase `godot_gdk` paths are never touched.

.PARAMETER Path
    Project root to migrate. Defaults to the current directory.

.PARAMETER Include
    Additional file extensions to process, e.g. `-Include .txt,.json`.

.PARAMETER IncludeMarkdown
    Also rewrite `.md` files. Off by default so prose about the Microsoft GDK
    product is not disturbed.

.PARAMETER RewriteBareGdkTypes
    Also rewrite unambiguous bare `GDK` *type* positions (`: GDK`, `is GDK`,
    `as GDK`, `extends GDK`) to `Xbox`. Singleton calls (`GDK.`) are still left alone.

.PARAMETER RenameProjectGdkNames
    Also rewrite `GDK*` / `Gdk*` identifiers that are *not* part of the addon API —
    a project's own `GdkAuth` autoload, for example. Off by default so the script
    only touches the surface v0.3.0 actually broke.

.PARAMETER RenameFiles
    Also rename `Gdk*.cs` files to `Xbox*.cs`.

.PARAMETER IncludeAddons
    Process the bundled `addons/godot_gdk*` directories too. Only useful if you
    have forked the addon itself; skipped by default.

.EXAMPLE
    .\tools\migrate_gdk_to_xbox.ps1 -Path C:\games\MyGame -WhatIf
    Dry run: prints every file and replacement that would be made.

.EXAMPLE
    .\tools\migrate_gdk_to_xbox.ps1 -Path C:\games\MyGame -RenameFiles
    Rewrites references and renames the C# facade-derived file names.
#>
[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [string]$Path = '.',

    [string[]]$Include = @(),

    [switch]$IncludeMarkdown,

    [switch]$RewriteBareGdkTypes,

    [switch]$RenameProjectGdkNames,

    [switch]$RenameFiles,

    [switch]$IncludeAddons
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$root = (Resolve-Path -LiteralPath $Path).Path

# --- What to scan -----------------------------------------------------------

$extensions = @('.gd', '.cs', '.tscn', '.tres', '.godot', '.cfg', '.csproj', '.sln', '.props')
if ($IncludeMarkdown) { $extensions += '.md' }
foreach ($ext in $Include) {
    $e = if ($ext.StartsWith('.')) { $ext } else { ".$ext" }
    if ($extensions -notcontains $e) { $extensions += $e }
}

$skipDirs = @('.git', '.godot', '.import', '.vs', 'bin', 'obj', 'build', 'node_modules', 'third_party')
$addonDirs = @('godot_gdk', 'godot_gdk_csharp', 'godot_gdk_editortools', 'godot_playfab', 'godot_playfab_csharp', 'godot_gameinput', 'gut', 'gut-4.5')

function Test-Skipped {
    param([string]$FullName)

    $rel = $FullName.Substring($root.Length).TrimStart('\', '/')
    $parts = $rel -split '[\\/]'

    foreach ($p in $parts) {
        if ($skipDirs -contains $p) { return $true }
    }
    if (-not $IncludeAddons) {
        for ($i = 0; $i -lt $parts.Length - 1; $i++) {
            if ($parts[$i] -eq 'addons' -and $addonDirs -contains $parts[$i + 1]) { return $true }
        }
    }
    return $false
}

# --- Replacement rules ------------------------------------------------------
# [regex] defaults to case-sensitive, unlike PowerShell's -replace operator.
# That matters: a case-insensitive rule would rewrite lowercase `godot_gdk` paths.

$opts = [System.Text.RegularExpressions.RegexOptions]::None

# The renamed addon API surface, listed explicitly. Anything not on these lists is
# left alone by default, so a project's own `Gdk`-prefixed names (say a `GdkAuth`
# autoload) survive unless -RenameProjectGdkNames is passed.

# GDScript / ClassDB types. `GDKBootstrap` is the autoload, not a class.
$gdscriptApiNames = @(
    'GDKAccessibility', 'GDKAchievement', 'GDKAchievements', 'GDKActivation',
    'GDKBootstrap', 'GDKCapture', 'GDKCaptureMetaData', 'GDKClosedCaptionProperties',
    'GDKDisplay', 'GDKDisplayTimeoutDeferral', 'GDKErrorReporting', 'GDKEvents',
    'GDKGameChat', 'GDKGameSave', 'GDKGameUI', 'GDKLauncher', 'GDKLeaderboard',
    'GDKLeaderboardColumn', 'GDKLeaderboardRow', 'GDKLeaderboards',
    'GDKMultiplayerActivity', 'GDKMultiplayerActivityInfo', 'GDKPackage',
    'GDKPackageMount', 'GDKPackageResourcePack', 'GDKPendingSignal', 'GDKPresence',
    'GDKPresenceRecord', 'GDKPrivacy', 'GDKProfile', 'GDKResult', 'GDKSocial',
    'GDKSocialFilter', 'GDKSocialGroup', 'GDKSocialUser', 'GDKSpeechSynthesizer',
    'GDKStats', 'GDKStore', 'GDKStoreLicenseStatus', 'GDKStringVerify', 'GDKSystem',
    'GDKTitleStorage', 'GDKTitleStorageBlobMetadata', 'GDKTitleStorageBlobMetadataResult',
    'GDKUser', 'GDKUserProfile', 'GDKUserSignOutDeferral', 'GDKUsers'
)

# C# facade types. The bare `Gdk` is the static entry-point class.
$csharpApiNames = @(
    'Gdk', 'GdkAccessibility', 'GdkAchievement', 'GdkAchievements', 'GdkActivation',
    'GdkCapture', 'GdkCaptureMetaData', 'GdkClosedCaptionProperties', 'GdkDisplay',
    'GdkDisplayTimeoutDeferral', 'GdkErrorReporting', 'GdkEvents', 'GdkGameChat',
    'GdkGameSave', 'GdkGameUi', 'GdkLauncher', 'GdkLeaderboard', 'GdkLeaderboardColumn',
    'GdkLeaderboardRow', 'GdkLeaderboards', 'GdkLeaderboardTypes',
    'GdkMultiplayerActivity', 'GdkMultiplayerActivityInfo', 'GdkObject', 'GdkPackage',
    'GdkPackageMount', 'GdkPackageResourcePack', 'GdkPackageTypes', 'GdkPresence',
    'GdkPresenceRecord', 'GdkPrivacy', 'GdkProfile', 'GdkResult', 'GdkRuntime',
    'GdkServiceBase', 'GdkSocial', 'GdkSocialFilter', 'GdkSocialGroup', 'GdkSocialUser',
    'GdkSocialTypes', 'GdkSpeechSynthesizer', 'GdkStats', 'GdkStore',
    'GdkStoreLicenseStatus', 'GdkStringVerify', 'GdkSystem', 'GdkTitleStorage',
    'GdkTitleStorageBlobMetadata', 'GdkTitleStorageBlobMetadataResult',
    'GdkTitleStorageTypes', 'GdkUser', 'GdkUserProfile', 'GdkUserSignOutDeferral',
    'GdkUsers'
)

function New-NameRegex {
    param([string[]]$Names)
    # Longest first so a short name never shadows a longer one.
    $sorted = $Names | Sort-Object -Property Length -Descending
    $alt = ($sorted | ForEach-Object { [regex]::Escape($_) }) -join '|'
    return [regex]::new("\b(?:$alt)\b", $opts)
}

$rxXboxServices = [regex]::new('\bGDKXboxServices\b', $opts)
$rxPkgEnv       = [regex]::new('\bGDKPKG_', $opts)
$rxGdscriptApi  = New-NameRegex -Names $gdscriptApiNames
$rxCsharpApi    = New-NameRegex -Names $csharpApiNames
$rxGodotGdkNs   = [regex]::new('\bGodotGdk\b', $opts)

# Opt-in sweeps for project-owned names that merely share the prefix.
$rxAnyGdkUpper  = [regex]::new('\bGDK(?<rest>[A-Z][A-Za-z0-9_]*)\b', $opts)
$rxAnyGdkMixed  = [regex]::new('\bGdk(?<rest>[A-Z][A-Za-z0-9_]*)\b', $opts)

# Tokens that look like renamed types but are not. `GDKDependencies` is the CMake
# helper and `GDKExportFeatures` is the export plugin's display name; both keep
# referring to the Microsoft GDK product.
$excludedTokens = @('GDKDependencies', 'GDKExportFeatures')

# Bare `GDK` in a type position. `GDK.` (singleton call) is excluded by construction.
$rxBareGdkType  = [regex]::new('(?<lead>:\s*|\bis\s+|\bas\s+|\bextends\s+)GDK\b(?!\s*\.)', $opts)

# Only these two shapes are genuinely ambiguous and worth a human's time. A bare
# "GDK" string passed to has_singleton()/singleton(), or prose mentioning the GDK
# product, is correct as-is and must stay quiet or the report is unreadable.
$ambiguousPatterns = @(
    [regex]::new('(?::\s*|\bis\s+|\bas\s+|\bextends\s+)GDK\b(?!\s*\.)', $opts),
    [regex]::new('(?:get_class\s*\(\s*\)\s*==\s*"GDK"|"GDK"\s*==\s*[A-Za-z_][A-Za-z0-9_]*\.get_class\s*\(\s*\)|is_class\s*\(\s*"GDK"\s*\))', $opts)
)

$prefixSwap = {
    param($m)
    if ($excludedTokens -contains $m.Value) { return $m.Value }
    return 'Xbox' + $m.Groups['rest'].Value
}

# `GDKResult` and `GdkResult` both drop a 3-character prefix.
$apiSwap = { param($m) 'Xbox' + $m.Value.Substring(3) }

function Convert-Content {
    param([string]$Text, [bool]$IsCSharp)

    $out = $Text

    # Order matters: the specific rules must run before any general prefix rule,
    # otherwise `GDKPKG_` would become `XboxPKG_`.
    $out = $rxXboxServices.Replace($out, 'XboxServices')
    $out = $rxPkgEnv.Replace($out, 'XBOXPKG_')
    $out = $rxGdscriptApi.Replace($out, $apiSwap)

    if ($IsCSharp) {
        # Must precede the bare `Gdk` rule so the namespace is not split.
        $out = $rxGodotGdkNs.Replace($out, 'GodotXbox')
        $out = $rxCsharpApi.Replace($out, $apiSwap)
    }

    if ($RenameProjectGdkNames) {
        $out = $rxAnyGdkUpper.Replace($out, $prefixSwap)
        $out = $rxAnyGdkMixed.Replace($out, $prefixSwap)
    }

    if ($RewriteBareGdkTypes) {
        $out = $rxBareGdkType.Replace($out, { param($m) $m.Groups['lead'].Value + 'Xbox' })
    }

    return $out
}

# --- Walk -------------------------------------------------------------------

$files = @(Get-ChildItem -LiteralPath $root -Recurse -File |
    Where-Object { $extensions -contains $_.Extension -and -not (Test-Skipped $_.FullName) })

$changedCount = 0
$ambiguous = New-Object System.Collections.Generic.List[string]

foreach ($file in $files) {
    $original = [System.IO.File]::ReadAllText($file.FullName)
    $isCSharp = @('.cs', '.csproj', '.sln', '.props') -contains $file.Extension

    $updated = Convert-Content -Text $original -IsCSharp $isCSharp

    # Collect bare `GDK` occurrences that are genuinely ambiguous, for review.
    $rel = $file.FullName.Substring($root.Length).TrimStart('\', '/')
    if ($file.Extension -ne '.cs') {
        $lineNo = 0
        foreach ($line in ($updated -split "`r?`n")) {
            $lineNo++
            foreach ($rx in $ambiguousPatterns) {
                if ($rx.IsMatch($line)) {
                    $ambiguous.Add(("{0}:{1}: {2}" -f $rel, $lineNo, $line.Trim()))
                    break
                }
            }
        }
    }

    if ($updated -ne $original) {
        if ($PSCmdlet.ShouldProcess($rel, 'rewrite GDK* references')) {
            # Preserve the file's existing BOM choice.
            $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
            $hasBom = $bytes.Length -ge 3 -and $bytes[0] -eq 0xEF -and $bytes[1] -eq 0xBB -and $bytes[2] -eq 0xBF
            $encoding = New-Object System.Text.UTF8Encoding($hasBom)
            [System.IO.File]::WriteAllText($file.FullName, $updated, $encoding)
        }
        Write-Host "  rewrote $rel" -ForegroundColor Green
        $changedCount++
    }
}

# --- Optional file renames --------------------------------------------------

$renamedCount = 0
if ($RenameFiles) {
    $csFiles = @(Get-ChildItem -LiteralPath $root -Recurse -File -Filter '*.cs' |
        Where-Object { -not (Test-Skipped $_.FullName) -and $_.BaseName -cmatch '^Gdk' })

    foreach ($file in $csFiles) {
        $newName = 'Xbox' + $file.BaseName.Substring(3) + $file.Extension
        $rel = $file.FullName.Substring($root.Length).TrimStart('\', '/')
        if ($PSCmdlet.ShouldProcess($rel, "rename to $newName")) {
            Move-Item -LiteralPath $file.FullName -Destination (Join-Path $file.DirectoryName $newName)
        }
        Write-Host "  renamed $rel -> $newName" -ForegroundColor Green
        $renamedCount++
    }
}

# --- Report -----------------------------------------------------------------

Write-Host ""
Write-Host "Scanned $($files.Count) file(s) under $root" -ForegroundColor Cyan
Write-Host "Rewrote $changedCount file(s)." -ForegroundColor Cyan
if ($RenameFiles) { Write-Host "Renamed $renamedCount file(s)." -ForegroundColor Cyan }

if ($ambiguous.Count -gt 0) {
    Write-Host ""
    Write-Host "Review these bare 'GDK' references by hand:" -ForegroundColor Yellow
    Write-Host "  The singleton is still named GDK, but its class is now Xbox." -ForegroundColor Yellow
    Write-Host "  Keep GDK for singleton access; use Xbox for type annotations and class checks." -ForegroundColor Yellow
    Write-Host ""
    foreach ($entry in $ambiguous) { Write-Host "  $entry" }
}

Write-Host ""
Write-Host "Next: open the project in the Godot editor. Scene and resource files reference" -ForegroundColor Cyan
Write-Host "script classes by name, so a missed rename fails at load time, not parse time." -ForegroundColor Cyan
