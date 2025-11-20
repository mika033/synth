# Snorkel Synth

A powerful VST3 synthesizer plugin with extensive modulation, sequencing, and arpeggiator capabilities.

## Features

### Sound Generation
- **3 Oscillators** with independent waveform morphing (Saw ↔ Square), coarse/fine tuning, and mix controls
- **Noise Oscillator** with white/pink noise blend and decay envelope
- **Sub-oscillator** capabilities (Osc 3 defaults to -12 semitones)
- **8-voice polyphony** for chords and sequences

### Filters & Envelopes
- **Resonant low-pass filter** with cutoff, resonance, and envelope modulation
- **Filter feedback** control for self-oscillation effects
- **Full ADSR envelopes** for both filter and amplitude
- **5 Saturation types**: Clean, Warm, Tube, Hard, Acid

### Modulation System
- **10 Dedicated LFOs** - one for each major parameter:
  - Cutoff, Resonance, Env Mod, Decay, Accent
  - Waveform, Sub Osc, Drive, Volume, Delay Mix
- **6 LFO waveforms**: Sine, Triangle, Saw Up, Saw Down, Square, Random
- **Tempo-synced rates**: 1/16 to 16/1 notes
- Independent rate, waveform, and depth control per LFO

### Sequencer & Arpeggiator
- **Melody Sequencer** with 16 steps
  - Per-step pitch, velocity, gate, slide, and accent
  - Multiple patterns with progression support
  - Randomization controls for creative variations
- **Arpeggiator** with multiple modes:
  - Up, Down, Up-Down, Random, As Played
  - Tempo-synced rates (1/32 to 1/1 with dotted and triplet)
  - Octave range (1-4 octaves)
  - Gate length, octave shift, and swing controls

### Effects & Processing
- **Tempo-synced delay** with dotted and triplet options
- **Drive/saturation** with multiple character types
- **Global octave shift** control
- **Master volume** and BPM controls

### Preset Management
- JSON-based preset system
- Factory presets included
- Save/load custom presets
- Progression patterns
- Sequencer presets with randomization configurations

## User Interface

The plugin features a tabbed interface with 6 main sections:

1. **Osc Tab** - 3 oscillators + noise, each with waveform, tuning, and mix
2. **Filter Tab** - Filter controls, ADSR, feedback, and saturation
3. **Modulation Tab** - 10 dedicated LFOs for deep modulation
4. **Sequencer Tab** - 16-step melody sequencer with progression
5. **Arpeggiator Tab** - Full-featured arpeggiator with swing and modes
6. **Progression Tab** - Pattern chaining and progression controls

## Build Instructions

### Windows

**Prerequisites:**
- CMake (https://cmake.org/download/)
- Visual Studio 2022 with "Desktop development with C++"

**Build:**
```bash
build.bat
```

### macOS/Linux

```bash
# Install dependencies
# macOS: brew install cmake
# Linux: sudo apt install cmake build-essential libasound2-dev libjack-dev

mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The plugin will be installed to your system's VST3 directory automatically.

## Installation

After building, the plugin is installed to:
- **macOS**: `~/Library/Audio/Plug-Ins/VST3/`
- **Windows**: `C:\Program Files\Common Files\VST3\`
- **Linux**: `~/.vst3/`

## Sound Design Tips

### Classic Acid Bass
- Set Osc 1 to sawtooth (left), moderate mix
- Cutoff: 500-1000 Hz
- Resonance: 0.7-0.9 (high)
- Env Mod: 0.5-0.7
- Short filter decay (0.1-0.3s)
- Add sub-osc for weight (0.4-0.6)
- Light drive (0.2-0.4)

### Deep Modulation
- Use dedicated LFOs from the Modulation tab
- Try slow LFO on cutoff + fast LFO on resonance
- Experiment with different waveforms per parameter
- Tempo-sync rates create rhythmic motion

### Sequenced Patterns
- Program melodies in the Sequencer tab
- Use slide for glide between notes
- Vary velocity and accent for dynamics
- Create pattern chains with Progression tab
- Use randomization for variations

## Customization

Edit `source/PluginProcessor.cpp` to customize:
- Plugin name and manufacturer
- Number of voices (polyphony)
- Default parameter values

## Technical Details

- **Sample rate**: Adapts to host DAW
- **Polyphony**: 8 voices (configurable)
- **Format**: VST3 + Standalone
- **DSP**: State-variable resonant filter with feedback
- **Envelopes**: Full ADSR for filter and amplitude

## License

Built with [JUCE Framework](https://juce.com/)
