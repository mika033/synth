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

REM Detect Visual Studio version
set GENERATOR=
if exist "%ProgramFiles%\Microsoft Visual Studio\2022" (
    set GENERATOR=Visual Studio 17 2022
    echo Found Visual Studio 2022
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2022" (
    set GENERATOR=Visual Studio 17 2022
    echo Found Visual Studio 2022
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2019" (
    set GENERATOR=Visual Studio 16 2019
    echo Found Visual Studio 2019
) else if exist "%ProgramFiles(x86)%\Microsoft Visual Studio\2019" (
    set GENERATOR=Visual Studio 16 2019
    echo Found Visual Studio 2019
) else (
    echo ERROR: Could not detect Visual Studio installation
    echo Please install Visual Studio 2019 or 2022
    cd ..
    pause
    exit /b 1
)

echo Generating Visual Studio solution...
echo Using generator: %GENERATOR%
echo.
cmake .. -G "%GENERATOR%" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to generate Visual Studio project
    echo.
    echo Make sure Visual Studio is installed with:
    echo - Desktop development with C++
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
