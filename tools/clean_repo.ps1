# XBOX Godot Sample Repo Cleanup
# Previews or removes ignored local artifacts from the current worktree.

param(
    [switch]$Apply
)

$ErrorActionPreference = "Stop"

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  XBOX Godot Sample Repo Cleanup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

Push-Location $RepoRoot
try {
    $gitCommand = Get-Command git -ErrorAction SilentlyContinue
    if ($null -eq $gitCommand) {
        throw "git is required to clean the repo worktree."
    }

    $gitArgs = @("clean")
    if ($Apply) {
        Write-Host "Removing ignored local artifacts from the worktree..." -ForegroundColor Yellow
        $gitArgs += "-fdX"
    } else {
        Write-Host "Preview only. Re-run with -Apply to delete ignored files." -ForegroundColor Yellow
        $gitArgs += "-ndX"
    }

    Write-Host ""
    & $gitCommand.Source @gitArgs
    if ($LASTEXITCODE -ne 0) {
        throw "git clean failed with exit code $LASTEXITCODE."
    }

    Write-Host ""
    if ($Apply) {
        Write-Host "Cleanup complete." -ForegroundColor Green
    } else {
        Write-Host "Preview complete." -ForegroundColor Green
    }
} finally {
    Pop-Location
}
