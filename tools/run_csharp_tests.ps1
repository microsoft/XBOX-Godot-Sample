#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Builds the C# facade libraries and runs the headless C# parity test suite.

.DESCRIPTION
    The C# track is validated with `dotnet build` + `dotnet test` rather than the
    GDScript parse gate / GUT (which are GDScript-only). This script:
      1. Builds the FacadeParity.Tests project once. Its ProjectReferences pull
         in all three facade class libraries (godot_gdk_csharp,
         godot_playfab_csharp, godot_gameinput_csharp), so this single build
         compiles the facades too -- a separate per-facade build loop would just
         compile them a second time.
      2. Runs the FacadeParity.Tests xUnit suite with `--no-build` (the facades
         and test assembly are already compiled from step 1). The suite reflects
         over the facade assemblies and asserts every native doc_classes member
         has a managed wrapper.

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

# Build the FacadeParity.Tests project once. Its ProjectReferences pull in all
# three facade class libraries (godot_gdk_csharp, godot_playfab_csharp,
# godot_gameinput_csharp), so this single build compiles the facades too --
# building them in a separate loop first would just compile them twice.
$testProject = 'tests/csharp/FacadeParity.Tests/FacadeParity.Tests.csproj'

Write-Host "==> Building $testProject (compiles the three facades via ProjectReferences)" -ForegroundColor Cyan
dotnet build (Join-Path $repoRoot $testProject) -v minimal --nologo
if ($LASTEXITCODE -ne 0) { throw "Build failed: $testProject" }

# Run with --no-build so the facades + test assembly are compiled only once.
Write-Host '==> Running FacadeParity.Tests (--no-build)' -ForegroundColor Cyan
dotnet test (Join-Path $repoRoot $testProject) --no-build -v minimal --nologo
if ($LASTEXITCODE -ne 0) { throw 'C# parity tests failed.' }

Write-Host 'C# facade build + parity tests passed.' -ForegroundColor Green
