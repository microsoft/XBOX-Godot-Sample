@echo off
setlocal EnableExtensions DisableDelayedExpansion
rem ============================================================================
rem  gdkpkg.cmd — Windows shell forwarder for the godot_gdk_packaging runner.
rem
rem  Usage:
rem      addons\godot_gdk_packaging\gdkpkg.cmd <verb> [--flag value] [...]
rem
rem  Behaviour:
rem    * Locates a Godot 4 executable via:
rem        1. GODOT_CONSOLE / GODOT_BIN / GODOT environment variables
rem        2. <script-dir>\..\..\sample\Godot*_console.exe (repo dev layout)
rem        3. CWD\Godot*_console.exe / CWD\Godot*.exe
rem        4. `where godot` / `where godot4`
rem    * Defaults the Godot project to the current working directory. Pass
rem      `--path <dir>` (passthrough flag) or `--godot <path>` (consumed
rem      here) to override.
rem    * Forwards every other argument to `run.gd` via Godot's `-s` switch
rem      so the script works even when the consumer hasn't yet run
rem      `godot --headless --import`.
rem    * Propagates the child Godot exit code (which mirrors the verb's
rem      PackagingResult.exit_code).
rem ============================================================================

set "SCRIPT_DIR=%~dp0"
set "GODOT_EXE="
set "PROJECT_PATH=%CD%"
set "ARG_COUNT=0"

rem ── Argument scan: pull off --godot and --path; forward everything else ────
:scan
if "%~1"=="" goto scan_done
if /i "%~1"=="--godot" goto scan_godot
if /i "%~1"=="--path" goto scan_path
set /a ARG_COUNT+=1
set "GDKPKG_ARG_%ARG_COUNT%=%~1"
shift
goto scan

:scan_godot
set "GODOT_EXE=%~2"
shift
shift
goto scan

:scan_path
set "PROJECT_PATH=%~2"
rem --path is consumed here and passed to Godot as its own --path flag below.
rem It is intentionally NOT forwarded to run.gd's user args.
shift
shift
goto scan

:scan_done

rem ── Godot discovery: env var → repo-local sample\ → CWD → PATH ───────────
if defined GODOT_EXE goto have_godot

if defined GODOT_CONSOLE call :try_candidate "%GODOT_CONSOLE%"
if defined GODOT_BIN call :try_candidate "%GODOT_BIN%"
if defined GODOT call :try_candidate "%GODOT%"

for %%P in ("%SCRIPT_DIR%..\..\sample" "%CD%") do (
    if exist "%%~P" (
        for %%F in ("%%~P\Godot*_console.exe") do call :try_candidate "%%~fF"
        for %%F in ("%%~P\Godot*.exe") do call :try_candidate "%%~fF"
    )
)

for %%C in (godot godot4) do (
    for /f "delims=" %%R in ('where %%C 2^>nul') do call :try_candidate "%%~R"
)

if defined GODOT_EXE goto have_godot

echo [gdkpkg] error: could not find a Godot 4 executable. 1>&2
echo [gdkpkg] set GODOT_CONSOLE / GODOT_BIN / GODOT, or pass --godot ^<path^>. 1>&2
exit /b 3

:have_godot
set "GDKPKG_GODOT_EXE=%GODOT_EXE%"
set "GDKPKG_PROJECT_PATH=%PROJECT_PATH%"
set "GDKPKG_ARG_COUNT=%ARG_COUNT%"

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -Command "$ErrorActionPreference = 'Stop'; $forwardArgs = @(); for ($i = 1; $i -le [int]$env:GDKPKG_ARG_COUNT; $i++) { $forwardArgs += [Environment]::GetEnvironmentVariable('GDKPKG_ARG_' + $i) }; & $env:GDKPKG_GODOT_EXE --headless --path $env:GDKPKG_PROJECT_PATH -s 'res://addons/godot_gdk_packaging/run.gd' -- @forwardArgs; if ($null -ne $global:LASTEXITCODE) { exit $global:LASTEXITCODE }; exit 0"
exit /b %ERRORLEVEL%

:try_candidate
if defined GODOT_EXE exit /b 0
if "%~1"=="" goto :eof
if exist "%~1" set "GODOT_EXE=%~1"
goto :eof
