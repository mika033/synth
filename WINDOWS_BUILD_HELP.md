# Windows Build Troubleshooting Guide

## Quick Start

1. Install CMake and Visual Studio (see README.md)
2. Run `build.bat`
3. If errors occur, check solutions below

## Common Issues

### "cmake is not recognized as a command"

**Problem:** CMake not in system PATH

**Solutions:**
1. Reinstall CMake and check "Add CMake to system PATH for all users"
2. Or add manually:
   - Search Windows for "Environment Variables"
   - Edit System PATH
   - Add: `C:\Program Files\CMake\bin`
3. Restart Command Prompt/PowerShell after changing PATH
4. Verify: `cmake --version` in cmd

### "CMAKE_C_COMPILER not set" or "nmake not found"

**Problem:** No C++ compiler found

**Solution:** Install Visual Studio Community 2022
- During install, check "Desktop development with C++"
- This includes MSVC compiler and build tools
- Restart after installation

**Alternative:** Use MinGW
- Install: https://www.mingw-w64.org/
- Or via Chocolatey: `choco install mingw`
- Run `build-mingw.bat` instead

### "Generator NMake Makefiles does not support platform specification"

**Problem:** CMake is detecting NMake instead of Visual Studio

**Solutions:**
1. **Easiest:** Run from "Developer Command Prompt for VS 2022"
   - Search in Start Menu for "Developer Command Prompt"
   - Run `build.bat` from there
2. The updated build.bat should auto-detect Visual Studio now
3. Or manually specify: `cmake .. -G "Visual Studio 17 2022" -A x64`

### CMake works in CMD but not PowerShell

**Problem:** PowerShell using cached PATH

**Solution:**
- Just use Command Prompt (cmd.exe) instead
- Or restart PowerShell
- Or run in PowerShell:
  ```powershell
  $env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
  ```

### Build fails with "Missing include files"

**Problem:** Visual Studio C++ tools not fully installed

**Solution:**
1. Open Visual Studio Installer
2. Modify your installation
3. Ensure "Desktop development with C++" is checked
4. Ensure these are selected:
   - MSVC C++ build tools
   - Windows 10/11 SDK
   - C++ CMake tools

### "Error opening file for writing"

**Problem:** Permission issues

**Solutions:**
- Run Command Prompt as Administrator
- Or build in a folder you own (like Documents)
- Avoid building in Program Files or system folders

### Build succeeds but VST3 not found in DAW

**Problem:** Plugin not in correct location or DAW hasn't rescanned

**Solutions:**
1. Check if VST3 was created:
   - Look in: `build\AcidSynth_artefacts\Release\VST3\`
2. Manually copy to: `C:\Program Files\Common Files\VST3\`
3. Rescan plugins in your DAW:
   - **Ableton Live**: Preferences → Plug-ins → Rescan
   - **FL Studio**: Options → Manage Plugins → Refresh
   - **Reaper**: Preferences → Plug-ins → VST → Re-scan

### Very slow first build

**Normal behavior:** First build downloads JUCE framework and compiles everything

- First build: 5-15 minutes (depending on CPU)
- Subsequent builds: 30 seconds - 2 minutes
- Be patient during first build!

## Using Visual Studio IDE

Prefer working in Visual Studio?

1. Run `generate-vs-project.bat`
2. Open `build\AcidSynth.sln`
3. Select "Release" configuration (dropdown at top)
4. Right-click "AcidSynth_VST3" → Build
5. Find VST3 in `build\AcidSynth_artefacts\Release\VST3\`

## Clean Build

If things are broken, try a clean build:

```cmd
rmdir /s /q build
build.bat
```

## Need More Help?

- Check CMake is version 3.15+: `cmake --version`
- Check Visual Studio installed: Look for "Visual Studio 2022" in Start Menu
- Check compiler available: `cl` (in Developer Command Prompt)

## Alternative: Pre-built Binaries

If building is too difficult, pre-built VST3 files may be available in the Releases section.
