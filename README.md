# Acid Synth VST

A minimal VST synthesizer plugin for creating classic Acid bass sounds, inspired by the legendary Roland TB-303.

## Features

- **Waveforms**: Sawtooth and Square wave oscillators
- **Resonant Filter**: Classic low-pass filter with resonance control
- **Filter Envelope**: Envelope modulation for that signature squelchy sound
- **Decay Control**: Adjustable envelope decay time
- **Accent**: Velocity-sensitive accent for dynamic expression
- **8-voice polyphony**: Play chords and sequences

## Controls

- **Cutoff**: Base filter cutoff frequency (20 Hz - 5000 Hz)
- **Resonance**: Filter resonance amount (0.0 - 0.95)
- **Env Mod**: Filter envelope modulation depth
- **Decay**: Filter envelope decay time
- **Accent**: Accent intensity
- **Waveform**: Switch between Sawtooth and Square wave

## Build Instructions

### Windows (Easiest Method)

**Prerequisites:**
1. Install **CMake**: https://cmake.org/download/ (check "Add to PATH" during install)
2. Install **Visual Studio Community 2022**: https://visualstudio.microsoft.com/downloads/
   - During installation, select "Desktop development with C++"

**Quick Build:**
1. Double-click `build.bat` in the synth folder
2. Wait for compilation (5-10 minutes first time)
3. Done! The VST3 will be installed automatically

**Alternative - Visual Studio IDE:**
1. Double-click `generate-vs-project.bat`
2. Open `build\AcidSynth.sln` in Visual Studio
3. Build → Build Solution (or press Ctrl+Shift+B)

**Using MinGW instead of Visual Studio:**
1. Install MinGW-w64 or via Chocolatey: `choco install mingw`
2. Double-click `build-mingw.bat`

### macOS/Linux

1. Install dependencies:
```bash
# macOS
brew install cmake

# Ubuntu/Debian
sudo apt install cmake build-essential libasound2-dev libjack-dev \
  libcurl4-openssl-dev libfreetype6-dev libx11-dev libxrandr-dev
```

2. Build:
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

### Installation

After building, the plugin will be automatically copied to your system's plugin directory:

- **macOS**: `~/Library/Audio/Plug-Ins/VST3/`
- **Windows**: `C:\Program Files\Common Files\VST3\`
- **Linux**: `~/.vst3/`

Alternatively, you can manually copy the built VST3 plugin from the build directory to your DAW's plugin folder.

## Usage in DAWs

### Ableton Live
1. Rescan your plugin folders (Preferences → Plug-ins → Rescan)
2. Create a new MIDI track
3. Add "Acid Synth" from the Instruments browser
4. Play some notes and tweak the filter controls!

### Other DAWs
Most DAWs will automatically detect the plugin after rescanning. Look for "Acid Synth" in your synthesizer/instrument list.

## Sound Design Tips

### Classic 303 Bass
- Use Sawtooth waveform
- Set Cutoff around 500-1000 Hz
- High Resonance (0.7-0.9)
- Moderate Env Mod (0.5-0.7)
- Short Decay (0.1-0.3s)

### Squelchy Lead
- Use Square waveform
- Lower Cutoff (300-500 Hz)
- Maximum Resonance
- High Env Mod (0.8-1.0)
- Longer Decay (0.5-1.0s)

### Accent Usage
- Use higher MIDI velocities (>100) to trigger accent
- Accent adds extra punch and filter opening to notes

## Technical Details

- Sample rate: Adapts to host DAW
- Polyphony: 8 voices
- Format: VST3
- DSP: State-variable resonant filter
- Envelope: Classic AD envelope with no sustain

## License

[Add your license here]

## Credits

Built with [JUCE Framework](https://juce.com/)
