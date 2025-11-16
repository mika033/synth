# Build Instructions

## Fast Builds (Recommended for development)

### Option 1: Use build-fast.bat (FASTEST)
```batch
build-fast.bat
```
- **Incremental builds only** - Only recompiles changed files
- **Parallel compilation** - Uses all CPU cores
- **Best for:** Making code changes and testing quickly
- **Speed:** 5-30 seconds for small changes

### Option 2: Use build.bat (default - incremental)
```batch
build.bat
```
- **Incremental builds by default** - Reuses previous build
- **Parallel compilation** - Uses all CPU cores
- **Best for:** Regular builds
- **Speed:** 10-60 seconds for small changes

## Clean Builds (when needed)

### When you need a clean build:
- After updating CMakeLists.txt
- After pulling new JUCE updates
- When strange build errors occur
- When dependencies change

```batch
build.bat clean
```
- Deletes entire build directory
- Reconfigures CMake from scratch
- Downloads JUCE if needed
- **Speed:** 2-10 minutes (first time or after clean)

## Build Speed Comparison

| Build Type | Command | Typical Time |
|------------|---------|--------------|
| **First build** | `build.bat` | 2-10 minutes |
| **Clean build** | `build.bat clean` | 2-10 minutes |
| **Incremental** | `build.bat` | 10-60 seconds |
| **Fast incremental** | `build-fast.bat` | 5-30 seconds |

## Tips for Faster Builds

1. **Always use incremental builds** unless you have a specific reason to clean
2. **Use build-fast.bat** when making code changes
3. **Only run `build.bat clean`** when absolutely necessary
4. **Use an SSD** if possible - significantly faster than HDD
5. **Close other programs** to free up CPU and disk I/O

## Output Location

After building, the VST3 plugin will be at:
```
build\AcidSynth_artefacts\Release\VST3\AcidSynth.vst3
```

It may also be automatically installed to:
```
%ProgramFiles%\Common Files\VST3\AcidSynth.vst3
```
