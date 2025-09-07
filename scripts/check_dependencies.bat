@echo off
title Aether Dependency Checker
color 07

echo Aether System Compatibility Check
echo ================================
echo.

REM Check Visual C++ Redistributable
echo Checking Visual C++ Runtime...
reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" >nul 2>&1
if %errorlevel% equ 0 (
    echo [OK] Visual C++ Runtime detected
) else (
    echo [MISSING] Visual C++ Redistributable required
    echo.
    echo Installing Microsoft Visual C++ Redistributable...
    
    REM Silent download with error handling
    powershell -WindowStyle Hidden -Command "try { Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vc_redist.x64.exe' -OutFile 'vc_redist.x64.exe' -UseBasicParsing } catch { exit 1 }" >nul 2>&1
    
    if exist "vc_redist.x64.exe" (
        echo Installing runtime components...
        vc_redist.x64.exe /quiet /norestart
        if %errorlevel% equ 0 (
            echo [OK] Runtime installed successfully
        ) else (
            echo [ERROR] Installation failed - manual installation required
        )
        del "vc_redist.x64.exe" >nul 2>&1
    ) else (
        echo [ERROR] Download failed - check internet connection
        echo Manual download: https://aka.ms/vs/17/release/vc_redist.x64.exe
    )
)

echo.
echo Checking system libraries...

REM Check Universal CRT
if exist "%SystemRoot%\System32\ucrtbase.dll" (
    echo [OK] Universal CRT available
) else (
    echo [WARNING] Universal CRT missing - Windows Update recommended
)

REM Check Common Controls
if exist "%SystemRoot%\System32\comctl32.dll" (
    echo [OK] Common Controls available
) else (
    echo [ERROR] Common Controls missing - system corruption detected
)

echo.
echo System check complete.
echo Aether is ready to run.
echo.
pause >nul
