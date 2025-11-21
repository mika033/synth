# Snorkel Synth - Claude Code Context

## Project Overview
VST3 synthesizer plugin built with JUCE framework. Includes oscillators, filters, modulation, sequencer, arpeggiator, chord progression, and drum machine.
The synth should be good for creating classic analog sounds, with a warm or an Acid (303-like) vibe.

## Development Workflow
Important: The user builds and tests! Claude should never do this!

## Build Commands
```bash
cmake --build build --config Release
```

## Naming conventions:
- Accent row: The row of controls ("Fill bars") below the main sequencer for accent modulation.
- Messaging area: The text area in the top bar for user feedback.
- Top bar: The horizontal bar at the top with Play, BPM, Root, Scale, Swing, Volume controls.

## Code Structure
- `include/` - Header files (.h)
- `source/` - Implementation files (.cpp)
- `data/presets/` - JSON preset files

### Key Files
- `PluginProcessor.h/cpp` - Audio processing, parameters, sequencer/arp/drum logic
- `PluginEditor.h/cpp` - Main UI, top bar controls (Play, BPM, Root, Scale, Swing, Volume)
- `MelodySequencerTab.h/cpp` - 16-step melodic sequencer with accent modulation
- `SequencerTab.h/cpp` - Arpeggiator
- `DrumTab.h/cpp` - Drum machine with pattern chain
- `OscTab.h/cpp`, `FilterTab.h/cpp`, `ModulationTab.h/cpp` - Synth controls

## UI Conventions
- Use `editor.showMessage("...")` for user feedback in the top bar message area
- Dials: `juce::Slider::RotaryVerticalDrag` with `NoTextBox`
- Standard dial size: 45x45 (like master volume)
- Accent control dials: 50x50
- Colors: Background `0xff2a2a2a`, buttons `0xff404040`, active `0xff00aa00`

## Parameter Naming
Parameters defined in PluginProcessor.h with constants like:
- `SEQ_GATE_ID` = "seqgate"
- Pattern: lowercase, no spaces (e.g., "seqaccentvol", "drumchainenabled")

## Audio Processing Notes
- Swing: Delays off-beat notes without changing note length
- Accent modulation: Multiplicative for volume/resonance, interpolation for drive
- Drums process AFTER delay (not through delay)
- One sequencer step per audio block to prevent timing issues on BPM changes
