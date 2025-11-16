@echo off
REM Acid Synth VST - Fast Incremental Build Script
REM Only rebuilds changed files

echo ========================================
echo Acid Synth VST - Fast Build
echo ========================================
echo.

REM Check if build directory exists
if not exist build (
    echo Build directory not found. Running full build first...
    call build.bat
    exit /b %ERRORLEVEL%
)

echo Using existing build directory (incremental build)
echo.

cd build

REM Build with parallel compilation (use all CPU cores)
echo Building with parallel compilation...
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
echo The VST3 plugin is at:
echo build\AcidSynth_artefacts\Release\VST3\
echo.
cd ..
pause
