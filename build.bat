@echo off
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

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM Create build directory
echo Creating build directory...
mkdir build
cd build

REM Configure with CMake (auto-detect Visual Studio)
echo.
echo Configuring project with CMake...
echo This will download JUCE framework (may take a few minutes)...
echo.
cmake .. -A x64

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake configuration failed!
    echo.
    echo Make sure you have Visual Studio installed with:
    echo - Desktop development with C++
    echo.
    echo Or use: cmake .. -G "MinGW Makefiles" if using MinGW
    cd ..
    pause
    exit /b 1
)

REM Build the project
echo.
echo Building Acid Synth VST (Release)...
echo This may take several minutes...
echo.
cmake --build . --config Release

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
