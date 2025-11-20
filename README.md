# Snorkel Synth

A VST3 synthesizer plugin with sequencer, arpeggiator, and modulation capabilities.

## Features

- Multiple oscillator waveforms with sub-oscillator
- Resonant filter with envelope and feedback
- Full ADSR envelopes for amplitude and filter
- LFO modulation for multiple parameters
- Built-in arpeggiator with swing and octave controls
- Melody sequencer with progression support
- Tempo-synced delay effect
- Saturation/drive controls
- Preset management

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

## License

Built with [JUCE Framework](https://juce.com/)
