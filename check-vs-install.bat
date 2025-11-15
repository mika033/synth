@echo off
REM Visual Studio Installation Checker
REM This script checks if Visual Studio C++ tools are properly installed

echo ========================================
echo Visual Studio C++ Tools Checker
echo ========================================
echo.

REM Check for cl.exe (MSVC compiler)
echo Checking for MSVC compiler (cl.exe)...
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [OK] MSVC compiler found:
    where cl
    cl 2>&1 | findstr "Microsoft"
    echo.
) else (
    echo [FAILED] MSVC compiler NOT found!
    echo This means C++ build tools are not installed or not in PATH
    echo.
)

REM Check for MSBuild
echo Checking for MSBuild...
where msbuild >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [OK] MSBuild found:
    where msbuild
    echo.
) else (
    echo [FAILED] MSBuild NOT found!
    echo.
)

REM Check CMake
echo Checking for CMake...
where cmake >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo [OK] CMake found:
    cmake --version
    echo.
) else (
    echo [FAILED] CMake NOT found!
    echo.
)

REM Check for Visual Studio directories
echo Checking Visual Studio installation directories...
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community" (
    echo [OK] Found VS 2022 Community at: %ProgramFiles%\Microsoft Visual Studio\2022\Community
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional" (
    echo [OK] Found VS 2022 Professional at: %ProgramFiles%\Microsoft Visual Studio\2022\Professional
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise" (
    echo [OK] Found VS 2022 Enterprise at: %ProgramFiles%\Microsoft Visual Studio\2022\Enterprise
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" (
    echo [OK] Found VS 2019
) else (
    echo [FAILED] Visual Studio directory not found!
)
echo.

REM Check for VC tools
echo Checking for Visual C++ tools directory...
if exist "%VCToolsInstallDir%" (
    echo [OK] VC Tools found at: %VCToolsInstallDir%
) else (
    echo [FAILED] VCToolsInstallDir not set!
    echo This variable should be set by Developer Command Prompt
)
echo.

echo ========================================
echo Summary
echo ========================================
echo.
echo If you see [FAILED] for cl.exe or MSBuild:
echo   1. Open Visual Studio Installer
echo   2. Click "Modify" on your Visual Studio installation
echo   3. Make sure these are CHECKED:
echo      - Desktop development with C++
echo      - MSVC v143 - VS 2022 C++ build tools
echo      - Windows 10/11 SDK
echo   4. Click Modify to install
echo   5. Restart and try building again
echo.
echo If running from regular cmd/PowerShell:
echo   - Search for "Developer Command Prompt for VS 2022" in Start Menu
echo   - Run the build from there
echo.
pause
