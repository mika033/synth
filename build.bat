@echo off
setlocal enabledelayedexpansion
REM Acid Synth VST Build Script for Windows
REM This script automates the build process

echo ========================================
echo Acid Synth VST - Build Script
echo ========================================
echo.

REM Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed or not in PATH
    echo Please install CMake from: https://cmake.org/download/
    echo Make sure to check "Add CMake to system PATH" during installation
    pause
    exit /b 1
)

echo CMake found:
cmake --version
echo.

REM Check if we should do a clean build
set CLEAN_BUILD=0
if "%1"=="clean" set CLEAN_BUILD=1

if %CLEAN_BUILD%==1 (
    if exist build (
        echo Performing clean build (deleting build directory)...
        rmdir /s /q build
    )
)

REM Create build directory if it doesn't exist
if not exist build (
    echo Creating build directory...
    mkdir build
) else (
    echo Using existing build directory (incremental build)...
)

cd build

REM Detect Visual Studio version
echo Detecting Visual Studio installation...
echo.

REM Try different Visual Studio versions
set GENERATOR=

REM Check VS 2022 in Program Files
if exist "%ProgramFiles%\Microsoft Visual Studio\2022" (
    set GENERATOR=Visual Studio 17 2022
    echo Found Visual Studio 2022
    goto generator_found
)

REM Check VS 2022 in Program Files x86 (use direct path construction)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2022" (
    set GENERATOR=Visual Studio 17 2022
    echo Found Visual Studio 2022
    goto generator_found
)

REM Check VS 2019 in Program Files
if exist "%ProgramFiles%\Microsoft Visual Studio\2019" (
    set GENERATOR=Visual Studio 16 2019
    echo Found Visual Studio 2019
    goto generator_found
)

REM Check VS 2019 in Program Files x86
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019" (
    set GENERATOR=Visual Studio 16 2019
    echo Found Visual Studio 2019
    goto generator_found
)

echo WARNING: Could not detect Visual Studio installation
echo Trying auto-detection...
set GENERATOR=

:generator_found

REM Configure with CMake
echo.
echo Configuring project with CMake...
echo This will download JUCE framework (may take a few minutes)...
echo.

if defined GENERATOR (
    echo Using generator: %GENERATOR%
    cmake .. -G "%GENERATOR%" -A x64
) else (
    echo Using auto-detected generator
    cmake ..
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ========================================
    echo ERROR: CMake configuration failed!
    echo ========================================
    echo.
    echo This usually means Visual Studio is not installed or not set up correctly.
    echo.
    echo Solutions:
    echo 1. Install Visual Studio Community 2022 from:
    echo    https://visualstudio.microsoft.com/downloads/
    echo    - During install, select "Desktop development with C++"
    echo.
    echo 2. Or use MinGW instead:
    echo    - Install MinGW-w64
    echo    - Run build-mingw.bat instead
    echo.
    echo 3. Or run from "Developer Command Prompt for VS 2022"
    echo    - Search for it in Start Menu
    echo    - Then run this script again
    echo.
    cd ..
    pause
    exit /b 1
)

REM Build the project
echo.
echo Building Acid Synth VST (Release)...
echo Using parallel compilation (all CPU cores)...
echo.
cmake --build . --config Release --parallel

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo The VST3 plugin should be installed at:
echo %ProgramFiles%\Common Files\VST3\AcidSynth.vst3
echo.
echo Or check: build\AcidSynth_artefacts\Release\VST3\
echo.
echo Rescan plugins in your DAW to use it!
echo ========================================
cd ..
pause
