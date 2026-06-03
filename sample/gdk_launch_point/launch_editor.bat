@echo off
cd /d "%~dp0"

set "GODOT_EXE=Godot_v4.6.1-stable_win64.exe"
if exist "%GODOT_EXE%" goto launch

REM Fallback to godot on PATH
where godot >nul 2>&1
if %ERRORLEVEL%==0 (
    set "GODOT_EXE=godot"
    goto launch
)

echo Could not find Godot.
echo   Looked for: %CD%\Godot_v4.6.1-stable_win64.exe
echo   And:        godot on PATH
exit /b 1

:launch
start "" "%GODOT_EXE%" --editor
