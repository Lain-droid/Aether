@echo off
echo Checking Aether Dependencies...
echo.

REM Check for Visual C++ Redistributable
echo [1/4] Checking Visual C++ Redistributable...
reg query "HKLM\SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" >nul 2>&1
if %errorlevel% equ 0 (
    echo ✓ Visual C++ Redistributable found
) else (
    echo ✗ Visual C++ Redistributable missing
    echo   Downloading vc_redist.x64.exe...
    powershell -Command "Invoke-WebRequest -Uri 'https://aka.ms/vs/17/release/vc_redist.x64.exe' -OutFile 'vc_redist.x64.exe'"
    echo   Installing...
    vc_redist.x64.exe /quiet /norestart
    del vc_redist.x64.exe
)

REM Check for Universal CRT
echo [2/4] Checking Universal CRT...
if exist "%SystemRoot%\System32\ucrtbase.dll" (
    echo ✓ Universal CRT found
) else (
    echo ✗ Universal CRT missing - Windows Update required
)

REM Check for RichEdit
echo [3/4] Checking RichEdit library...
if exist "%SystemRoot%\System32\riched20.dll" (
    echo ✓ RichEdit library found
) else (
    echo ✗ RichEdit library missing
)

REM Check for Common Controls
echo [4/4] Checking Common Controls...
if exist "%SystemRoot%\System32\comctl32.dll" (
    echo ✓ Common Controls found
) else (
    echo ✗ Common Controls missing
)

echo.
echo Dependency check complete!
echo Ready to run Aether.
pause
