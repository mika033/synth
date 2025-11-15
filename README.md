# Acid Synth VST

A minimal VST synthesizer plugin for creating classic Acid bass sounds, inspired by the legendary Roland TB-303.

## Features

- **Waveforms**: Sawtooth and Square wave oscillators
- **Resonant Filter**: Classic low-pass filter with resonance control
- **Filter Envelope**: Envelope modulation for that signature squelchy sound
- **Decay Control**: Adjustable envelope decay time
- **Accent**: Velocity-sensitive accent for dynamic expression
- **Tempo-Synced LFO**: Assignable LFO with tempo sync for modulation effects
- **Tempo-Synced Delay**: Classic delay effect synchronized to your DAW's tempo
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

### LFO Section
- **LFO Rate**: Tempo-synced rate (1/16, 1/8, 1/4, 1/2, 1/1 notes)
  - Automatically syncs to your DAW's tempo
  - 1/4 note = one LFO cycle every quarter note
- **LFO Dest**: Modulation destination (Off, Cutoff, Resonance, Volume)
  - Off: LFO is disabled
  - Cutoff: Modulates filter cutoff frequency
  - Resonance: Modulates filter resonance amount
  - Volume: Modulates output volume (tremolo effect)
- **LFO Depth**: Amount of modulation (0.0 - 1.0)
  - Controls how much the LFO affects the selected destination

### Delay Section
- **Delay Time**: Tempo-synced delay time with dotted and triplet options
  - Straight: 1/16, 1/8, 1/4, 1/2, 1/1 notes
  - Dotted: 1/16., 1/8., 1/4., 1/2. (1.5x length)
  - Triplet: 1/16T, 1/8T, 1/4T (2/3 length)
  - Automatically syncs to your DAW's tempo
  - Example: 1/4. = dotted quarter note delay (classic dub delay)
- **Delay FB**: Feedback amount (0.0 - 0.95) - controls how many repeats
  - At 0.0: Single echo only
  - At 0.5: Several repeats
  - At 0.95: Very long decay, almost infinite
- **Delay Mix**: Wet/dry mix (0.0 - 1.0)
  - At 0.0: No delay (dry signal only)
  - At 0.5: Equal mix of dry and delayed signal
  - At 1.0: Only delayed signal

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

## Customization

Want to customize the plugin name, company, or settings? All configuration is centralized at the top of these files:

### CMakeLists.txt (Build Configuration)

Edit lines 7-18 to customize:

```cmake
set(PLUGIN_NAME "AcidSynth")              # Internal name (no spaces)
set(PLUGIN_VERSION "1.0.0")                # Version number
set(PLUGIN_DISPLAY_NAME "Acid Synth")     # Name shown in DAW
set(COMPANY_NAME "AcidLab")                # Your company name
set(PLUGIN_MANUFACTURER_CODE "Acdl")       # 4-char manufacturer ID
set(PLUGIN_CODE "Acd1")                    # 4-char plugin ID
set(PLUGIN_FORMATS "VST3" "Standalone")    # Build formats
set(INSTALL_TO_SYSTEM TRUE)                # Auto-install to VST folder
```

**Important:** PLUGIN_MANUFACTURER_CODE and PLUGIN_CODE must be unique 4-character codes if you distribute your plugin!

### source/PluginProcessor.cpp (Audio Configuration)

Edit lines 9-28 to customize audio behavior:

```cpp
// Plugin Identity
static constexpr const char* kPluginName = "Acid Synth";
static constexpr const char* kPluginVersion = "1.0.0";
static constexpr const char* kManufacturerName = "AcidLab";

// Audio Configuration
static constexpr int kNumVoices = 8;  // Polyphony (1-32 typical)

// Default Parameter Values
namespace Defaults {
    static constexpr float kCutoff = 1000.0f;      // Hz
    static constexpr float kResonance = 0.7f;      // 0-1
    static constexpr float kEnvMod = 0.5f;         // 0-1
    static constexpr float kDecay = 0.3f;          // seconds
    static constexpr float kAccent = 0.5f;         // 0-1
    static constexpr int   kWaveform = 0;          // 0=saw, 1=square
    static constexpr float kSubOsc = 0.5f;         // 0-1
    static constexpr float kDrive = 0.0f;          // 0-1
    static constexpr float kVolume = 0.7f;         // 0-1
    static constexpr int   kLFORate = 2;           // 1/4 note
    static constexpr int   kLFODest = 0;           // Off
    static constexpr float kLFODepth = 0.5f;       // 0-1
    static constexpr int   kDelayTime = 2;         // 1/4 note
    static constexpr float kDelayFeedback = 0.3f;  // 0-0.95
    static constexpr float kDelayMix = 0.0f;       // 0-1 (off by default)
}
```

After changing these, rebuild the plugin for changes to take effect.

## Technical Details

- Sample rate: Adapts to host DAW
- Polyphony: 8 voices (configurable via kNumVoices)
- Format: VST3, Standalone (configurable via PLUGIN_FORMATS)
- DSP: State-variable resonant filter
- Envelope: Classic AD envelope with no sustain

## License

[Add your license here]

## Credits

Built with [JUCE Framework](https://juce.com/)
