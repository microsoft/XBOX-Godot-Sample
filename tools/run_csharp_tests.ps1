#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Builds the C# facade libraries and runs the headless C# parity test suite.

.DESCRIPTION
    The C# track is validated with `dotnet build` + `dotnet test` rather than the
    GDScript parse gate / GUT (which are GDScript-only). This script:
      1. Builds the three facade class libraries (godot_gdk_csharp,
         godot_playfab_csharp, godot_gameinput_csharp).
      2. Runs the FacadeParity.Tests xUnit suite, which reflects over the facade
         assemblies and asserts every native doc_classes member has a managed
         wrapper.

    These tests run fully headless (no Godot _mono editor required), because they
    only inspect managed metadata. In-engine GoDotTest hosts (which exercise the
    live native singletons) require a Godot .NET editor and are run separately.

.NOTES
    Run from anywhere; paths are resolved relative to the repo root.
#>
[CmdletBinding()]
param()

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot

$facades = @(
    'addons/godot_gdk_csharp',
    'addons/godot_playfab_csharp',
    'addons/godot_gameinput_csharp'
)

foreach ($facade in $facades) {
    Write-Host "==> Building $facade" -ForegroundColor Cyan
    dotnet build (Join-Path $repoRoot $facade) -v minimal --nologo
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $facade" }
}

Write-Host '==> Running FacadeParity.Tests' -ForegroundColor Cyan
dotnet test (Join-Path $repoRoot 'tests/csharp/FacadeParity.Tests') -v minimal --nologo
if ($LASTEXITCODE -ne 0) { throw 'C# parity tests failed.' }

Write-Host 'C# facade build + parity tests passed.' -ForegroundColor Green
