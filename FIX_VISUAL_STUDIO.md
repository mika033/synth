# How to Fix Visual Studio for Building VST Plugins

If you're getting **"CMAKE_C_COMPILER not set"** errors even with Visual Studio installed, the C++ build tools are missing or not configured correctly.

## Step 1: Check What's Missing

Run this script to diagnose the problem:
```
check-vs-install.bat
```

This will tell you exactly what's missing.

## Step 2: Fix Visual Studio Installation

### Option A: Modify Existing Installation (Recommended)

1. **Open Visual Studio Installer**
   - Search "Visual Studio Installer" in Start Menu
   - Or go to: Control Panel → Programs → Visual Studio

2. **Click "Modify"** on your Visual Studio 2022 installation

3. **In the Workloads tab, check:**
   - ✅ **Desktop development with C++** (THIS IS CRITICAL!)

4. **In the Individual Components tab, verify these are checked:**
   - ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools (Latest)
   - ✅ C++ CMake tools for Windows
   - ✅ Windows 10 SDK (10.0.19041.0 or newer)
   - ✅ C++ core features

5. **Click "Modify"** button (bottom right)
   - This will download and install the missing components
   - Size: ~2-5 GB
   - Time: 10-30 minutes

6. **Restart your computer** after installation

### Option B: Complete Reinstall

If modifying doesn't work:

1. Uninstall Visual Studio completely
2. Download fresh installer: https://visualstudio.microsoft.com/downloads/
3. Run installer
4. Select **"Desktop development with C++"** workload
5. Install

## Step 3: Verify Installation

After installation/modification:

1. **Close all terminals/command prompts**

2. **Open "Developer Command Prompt for VS 2022"**
   - Search for it in Start Menu
   - Should be under "Visual Studio 2022" folder

3. **Test the compiler:**
   ```cmd
   cl
   ```
   - Should show: "Microsoft (R) C/C++ Optimizing Compiler Version..."
   - If you get "cl is not recognized", installation is still incomplete

4. **Run the diagnostic:**
   ```cmd
   cd C:\Users\mkais\PycharmProjects\synth
   check-vs-install.bat
   ```
   - All checks should show [OK]

## Step 4: Build the Plugin

Once all checks pass:

```cmd
cd C:\Users\mkais\PycharmProjects\synth
rmdir /s /q build
build.bat
```

## Still Not Working?

### Alternative 1: Use MinGW (Much Simpler)

If Visual Studio is too problematic, use MinGW instead:

1. **Install Chocolatey** (package manager for Windows):
   - Open PowerShell as Administrator
   - Run:
     ```powershell
     Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
     ```

2. **Install MinGW:**
   ```powershell
   choco install mingw -y
   ```

3. **Close and reopen PowerShell**, then:
   ```cmd
   cd C:\Users\mkais\PycharmProjects\synth
   build-mingw.bat
   ```

### Alternative 2: Manual Download of MinGW

1. Download: https://github.com/niXman/mingw-builds-binaries/releases
   - Get: `x86_64-13.2.0-release-posix-seh-ucrt-rt_v11-rev0.7z`
2. Extract to `C:\mingw64`
3. Add to PATH: `C:\mingw64\bin`
4. Restart terminal and run `build-mingw.bat`

## Common Issues

### "Running from wrong prompt"
- Regular cmd/PowerShell won't work
- Must use **"Developer Command Prompt for VS 2022"**

### "Visual Studio Installer can't find installation"
- Visual Studio might be corrupted
- Do a complete reinstall

### "Download is too large"
- C++ build tools are ~2-5 GB
- This is normal
- Ensure you have enough disk space

## Quick Test Commands

In Developer Command Prompt:

```cmd
REM Test compiler
cl

REM Test CMake
cmake --version

REM Test build tools
where msbuild
```

All three should work. If any fail, the installation is incomplete.
