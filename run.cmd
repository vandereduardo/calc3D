@echo off
setlocal
set "appDir=%~dp0build\windows-msvc\Release"
set "appExe=%appDir%\calc3D.exe"
if not exist "%appExe%" (
    echo O aplicativo nao foi compilado ainda.
    echo Execute .\scripts\build.ps1 no PowerShell para cria-lo.
    pause
    exit /b 1
)
start "" /D "%appDir%" "%appExe%"
