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

### Filter Section
- **Cutoff**: Base filter cutoff frequency (20 Hz - 5000 Hz)
- **Resonance**: Filter resonance amount (0.0 - 0.95) - higher values = more resonance
- **Env Mod**: Filter envelope modulation depth - how much the envelope affects the filter
- **Decay**: Filter envelope decay time - how quickly the filter closes
- **Accent**: Velocity sensitivity (0.0 - 1.0) - controls how much note velocity affects filter brightness
  - At 0.0: All notes have same brightness regardless of velocity
  - At 1.0: High velocity notes open filter more, low velocity notes less

### Tone Shaping Section
- **Sub Osc**: Sub-oscillator mix (0.0 - 1.0) - adds a sine wave one octave below for extra bass weight
  - At 0.0: No sub-oscillator, just the main oscillator
  - At 0.5: Balanced mix of main and sub (recommended)
  - At 1.0: Only sub-oscillator (very deep bass)
- **Drive**: Saturation/distortion amount (0.0 - 1.0) - adds harmonics and aggression
  - At 0.0: Clean sound
  - At 0.5: Slight warmth and harmonics
  - At 1.0: Heavy saturation, very aggressive
- **Volume**: Master output level (0.0 - 1.0)
- **Waveform**: Switch between Sawtooth (bright, classic) and Square wave (hollow, dark)

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

### Classic 303 Bass (Fat & Punchy)
- **Waveform**: Sawtooth
- **Cutoff**: 500-1000 Hz
- **Resonance**: 0.7-0.9 (high!)
- **Env Mod**: 0.5-0.7
- **Decay**: 0.1-0.3s (short and punchy)
- **Accent**: 0.6-0.8 (moderate velocity sensitivity)
- **Sub Osc**: 0.4-0.6 (adds weight without muddiness)
- **Drive**: 0.2-0.4 (slight analog warmth)
- **Volume**: 0.6-0.8

### Squelchy Lead (Maximum Expression)
- **Waveform**: Square
- **Cutoff**: 300-500 Hz (lower)
- **Resonance**: 0.85-0.95 (maximum!)
- **Env Mod**: 0.8-1.0 (lots of sweep)
- **Decay**: 0.5-1.0s (longer)
- **Accent**: 0.7-1.0 (high sensitivity for expression)
- **Sub Osc**: 0.2-0.3 (just a hint)
- **Drive**: 0.5-0.7 (add harmonics and aggression)
- **Volume**: 0.5-0.7

### Deep Rumble (Sub-Heavy)
- **Waveform**: Sawtooth or Square
- **Cutoff**: 200-400 Hz (very low)
- **Resonance**: 0.5-0.7 (moderate)
- **Env Mod**: 0.3-0.5 (subtle)
- **Decay**: 0.4-0.8s
- **Accent**: 0.3-0.5 (low for consistency)
- **Sub Osc**: 0.7-0.9 (heavy sub presence!)
- **Drive**: 0.1-0.3 (keep it clean for maximum low-end)
- **Volume**: 0.4-0.6 (turn down to prevent clipping)

### Aggressive Distorted Lead
- **Waveform**: Square
- **Cutoff**: 800-1500 Hz
- **Resonance**: 0.8-0.95
- **Env Mod**: 0.7-0.9
- **Decay**: 0.2-0.5s
- **Accent**: 0.8-1.0 (maximum dynamics)
- **Sub Osc**: 0.0-0.2 (keep it tight, not boomy)
- **Drive**: 0.7-1.0 (heavy saturation!)
- **Volume**: 0.5-0.7 (compensate for drive gain)

### Using Accent for Expression
- **Accent at 0.0**: All notes sound the same - good for consistent basslines
- **Accent at 0.5**: Moderate expression - subtle velocity response
- **Accent at 1.0**: Maximum expression - play dynamics matter a lot!
- **Pro tip**: Use varying MIDI velocities (60-127) with Accent at 0.7+ for dynamic acid lines
- Higher velocity notes will have brighter attack and more filter sweep

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
