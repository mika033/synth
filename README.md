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

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler (GCC, Clang, MSVC)
- Git

### Building

1. Clone the repository:
```bash
git clone <repository-url>
cd synth
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Configure with CMake:
```bash
cmake ..
```

4. Build:
```bash
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
