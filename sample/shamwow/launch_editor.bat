@echo off
cd /d "%~dp0"
set "GODOT_EXE=Godot_v4.6.1-stable_win64.exe"
if exist "%GODOT_EXE%" goto launch

set "GODOT_EXE=..\sample\Godot_v4.6.1-stable_win64.exe"
if exist "%GODOT_EXE%" goto launch

echo Could not find Godot_v4.6.1-stable_win64.exe in:
echo   %CD%
echo   %CD%\..\sample
exit /b 1

:launch
start "" "%GODOT_EXE%" --editor
