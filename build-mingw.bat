@echo off
REM Acid Synth VST Build Script for Windows (MinGW version)
REM Use this if you have MinGW installed instead of Visual Studio

echo ========================================
echo Acid Synth VST - MinGW Build Script
echo ========================================
echo.

REM Check if CMake is installed
where cmake >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake is not installed or not in PATH
    pause
    exit /b 1
)

REM Check if MinGW is installed
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MinGW g++ is not installed or not in PATH
    echo Please install MinGW or MinGW-w64
    pause
    exit /b 1
)

echo CMake found:
cmake --version
echo.
echo MinGW found:
g++ --version | findstr "g++"
echo.

REM Clean previous build
if exist build (
    echo Cleaning previous build...
    rmdir /s /q build
)

REM Create build directory
mkdir build
cd build

REM Configure with MinGW
echo.
echo Configuring project with MinGW...
echo.
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed!
    cd ..
    pause
    exit /b 1
)

REM Build
echo.
echo Building Acid Synth VST...
echo.
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed!
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
cd ..
pause
