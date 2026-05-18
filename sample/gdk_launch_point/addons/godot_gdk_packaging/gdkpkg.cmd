@echo off
setlocal EnableDelayedExpansion
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
set "FORWARD_ARGS="

rem ── Argument scan: pull off --godot and --path; forward everything else ────
:scan
if "%~1"=="" goto scan_done
if /i "%~1"=="--godot" (
    set "GODOT_EXE=%~2"
    shift
    shift
    goto scan
)
if /i "%~1"=="--path" (
    set "PROJECT_PATH=%~2"
    rem --path is consumed here and passed to Godot as its own --path flag below.
    rem It is intentionally NOT forwarded to run.gd's user args.
    shift
    shift
    goto scan
)
if not defined FORWARD_ARGS (
    set "FORWARD_ARGS=%~1"
) else (
    set FORWARD_ARGS=!FORWARD_ARGS! %1
)
shift
goto scan
:scan_done

rem ── Godot discovery: env var → repo-local sample\ → CWD → PATH ───────────
if defined GODOT_EXE goto have_godot

for %%V in (GODOT_CONSOLE GODOT_BIN GODOT) do (
    if defined %%V (
        call set "_CANDIDATE=%%%%V%%"
        call :try_candidate "!_CANDIDATE!"
        if defined GODOT_EXE goto have_godot
    )
)

for %%P in ("%SCRIPT_DIR%..\..\sample" "%CD%") do (
    if exist "%%~P" (
        for %%F in ("%%~P\Godot*_console.exe") do (
            call :try_candidate "%%~fF"
            if defined GODOT_EXE goto have_godot
        )
        for %%F in ("%%~P\Godot*.exe") do (
            call :try_candidate "%%~fF"
            if defined GODOT_EXE goto have_godot
        )
    )
)

for %%C in (godot godot4) do (
    for /f "delims=" %%R in ('where %%C 2^>nul') do (
        call :try_candidate "%%~R"
        if defined GODOT_EXE goto have_godot
    )
)

echo [gdkpkg] error: could not find a Godot 4 executable. 1>&2
echo [gdkpkg] set GODOT_CONSOLE / GODOT_BIN / GODOT, or pass --godot ^<path^>. 1>&2
exit /b 3

:have_godot
"%GODOT_EXE%" --headless --path "%PROJECT_PATH%" -s res://addons/godot_gdk_packaging/run.gd -- %FORWARD_ARGS%
exit /b %ERRORLEVEL%

:try_candidate
if "%~1"=="" goto :eof
if exist "%~1" set "GODOT_EXE=%~1"
goto :eof
