@echo off
REM Generate Visual Studio Solution
REM This creates a .sln file you can open in Visual Studio

echo ========================================
echo Generating Visual Studio Project
echo ========================================
echo.

REM Clean previous build
if exist build (
    rmdir /s /q build
)

mkdir build
cd build

echo Generating Visual Studio solution...
echo.
cmake .. -A x64

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to generate Visual Studio project
    cd ..
    pause
    exit /b 1
)

echo.
echo ========================================
echo Visual Studio solution generated!
echo ========================================
echo.
echo Open: build\AcidSynth.sln
echo.
echo Then build the "AcidSynth_VST3" project in Release mode
echo ========================================
cd ..
pause
