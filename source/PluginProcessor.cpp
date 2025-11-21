#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// CONFIGURATION - Edit these constants to customize the plugin
//==============================================================================

// Plugin Identity
static constexpr const char* kPluginName = "Snorkel Synth";
static constexpr const char* kPluginVersion = "1.0.0";
static constexpr const char* kManufacturerName = "SnorkelLab";

// Audio Configuration
static constexpr int kNumVoices = 8;  // Number of simultaneous notes (polyphony)

// Default Parameter Values
namespace Defaults
{
    static constexpr float kCutoff = 1000.0f;
    static constexpr float kResonance = 0.7f;
    static constexpr float kEnvMod = 0.5f;
    static constexpr float kDecay = 0.3f;
    static constexpr float kAccent = 0.5f;
    static constexpr float kWaveform = 0.0f;  // 0.0=saw, 1.0=square, morph in between
    static constexpr float kSubOsc = 0.5f;
    static constexpr float kDrive = 0.0f;
    static constexpr float kVolume = 0.7f;
    static constexpr int   kDelayTime = 1;  // 1/8 note (index 1)
    static constexpr float kDelayFeedback = 0.3f;
    static constexpr float kDelayMix = 0.0f; // Off by default

    // Dedicated LFO defaults (each parameter has its own LFO)
    static constexpr int   kLFORate = 6;     // Default: 1/1 (whole note)
    static constexpr int   kLFOWaveform = 0; // Default: Sine wave
    static constexpr float kLFODepth = 0.0f; // Default: Off
}

//==============================================================================
// LFO Rate and Waveform Options
static const juce::StringArray getLFORateOptions()
{
    return juce::StringArray{"1/16", "1/8", "1/4", "1/3", "1/2", "3/4", "1/1", "3/2", "2/1", "3/1", "4/1", "6/1", "8/1", "12/1", "16/1"};
}

static const juce::StringArray getLFOWaveformOptions()
{
    return juce::StringArray{"Sine", "Triangle", "Saw Up", "Saw Down", "Square", "Random"};
}

//==============================================================================
SnorkelSynthAudioProcessor::SnorkelSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS",
                {
                    // Main Parameters
                    std::make_unique<juce::AudioParameterFloat>(
                        CUTOFF_ID, "Cutoff",
                        juce::NormalisableRange<float>(20.0f, 5000.0f, 1.0f, 0.3f),
                        Defaults::kCutoff),

                    std::make_unique<juce::AudioParameterFloat>(
                        RESONANCE_ID, "Resonance",
                        juce::NormalisableRange<float>(0.0f, 1.5f, 0.01f),
                        Defaults::kResonance),

                    std::make_unique<juce::AudioParameterFloat>(
                        ENV_MOD_ID, "Env Mod",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kEnvMod),

                    std::make_unique<juce::AudioParameterFloat>(
                        ACCENT_ID, "Accent",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kAccent),

                    // Filter ADSR Parameters
                    std::make_unique<juce::AudioParameterFloat>(
                        FILTER_ATTACK_ID, "Filter Attack",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f, 0.5f),
                        0.003f), // Default: 3ms fast attack

                    std::make_unique<juce::AudioParameterFloat>(
                        FILTER_DECAY_ID, "Filter Decay",
                        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f),
                        0.3f), // Default: 300ms

                    std::make_unique<juce::AudioParameterFloat>(
                        FILTER_SUSTAIN_ID, "Filter Sustain",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: no sustain (classic 303 behavior)

                    std::make_unique<juce::AudioParameterFloat>(
                        FILTER_RELEASE_ID, "Filter Release",
                        juce::NormalisableRange<float>(0.0f, 5.0f, 0.001f, 0.5f),
                        0.1f), // Default: 100ms

                    // Amplitude ADSR Parameters
                    std::make_unique<juce::AudioParameterFloat>(
                        AMP_ATTACK_ID, "Amp Attack",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.001f, 0.5f),
                        0.003f), // Default: 3ms fast attack

                    std::make_unique<juce::AudioParameterFloat>(
                        AMP_DECAY_ID, "Amp Decay",
                        juce::NormalisableRange<float>(0.001f, 5.0f, 0.001f, 0.5f),
                        0.3f), // Default: 300ms

                    std::make_unique<juce::AudioParameterFloat>(
                        AMP_SUSTAIN_ID, "Amp Sustain",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: no sustain (classic 303 behavior)

                    std::make_unique<juce::AudioParameterFloat>(
                        AMP_RELEASE_ID, "Amp Release",
                        juce::NormalisableRange<float>(0.0f, 5.0f, 0.001f, 0.5f),
                        0.1f), // Default: 100ms

                    // Oscillator 1
                    std::make_unique<juce::AudioParameterFloat>(
                        OSC1_WAVE_ID, "Osc 1 Wave",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f), // Default: Sawtooth (middle)

                    std::make_unique<juce::AudioParameterInt>(
                        OSC1_COARSE_ID, "Osc 1 Coarse",
                        -24, 24, 0), // Range: -24 to +24 semitones, default: 0

                    std::make_unique<juce::AudioParameterFloat>(
                        OSC1_FINE_ID, "Osc 1 Fine",
                        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
                        0.0f), // Range: -100 to +100 cents, default: 0

                    std::make_unique<juce::AudioParameterFloat>(
                        OSC1_MIX_ID, "Osc 1 Mix",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.7f), // Default: 0.7

                    // Oscillator 2
                    std::make_unique<juce::AudioParameterFloat>(
                        OSC2_WAVE_ID, "Osc 2 Wave",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f), // Default: Sawtooth (middle)

                    std::make_unique<juce::AudioParameterInt>(
                        OSC2_COARSE_ID, "Osc 2 Coarse",
                        -24, 24, 0), // Range: -24 to +24 semitones, default: 0

                    std::make_unique<juce::AudioParameterFloat>(
                        OSC2_FINE_ID, "Osc 2 Fine",
                        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
                        0.0f), // Range: -100 to +100 cents, default: 0

                    std::make_unique<juce::AudioParameterFloat>(
                        OSC2_MIX_ID, "Osc 2 Mix",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: 0 (off for old presets)

                    // Oscillator 3 (sub osc by default)
                    std::make_unique<juce::AudioParameterFloat>(
                        OSC3_WAVE_ID, "Osc 3 Wave",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: Sine (left)

                    std::make_unique<juce::AudioParameterInt>(
                        OSC3_COARSE_ID, "Osc 3 Coarse",
                        -24, 24, -12), // Range: -24 to +24 semitones, default: -12 (one octave down)

                    std::make_unique<juce::AudioParameterFloat>(
                        OSC3_FINE_ID, "Osc 3 Fine",
                        juce::NormalisableRange<float>(-100.0f, 100.0f, 1.0f),
                        0.0f), // Range: -100 to +100 cents, default: 0

                    std::make_unique<juce::AudioParameterFloat>(
                        OSC3_MIX_ID, "Osc 3 Mix",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f), // Default: 0.5 (matches old subosc default)

                    // Noise oscillator
                    std::make_unique<juce::AudioParameterFloat>(
                        NOISE_TYPE_ID, "Noise Type",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: 0 (white noise)

                    std::make_unique<juce::AudioParameterFloat>(
                        NOISE_DECAY_ID, "Noise Decay",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
                        2.0f), // Default: 2.0 (right = no decay, sustained noise)

                    std::make_unique<juce::AudioParameterFloat>(
                        NOISE_MIX_ID, "Noise Mix",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: 0 (off)

                    std::make_unique<juce::AudioParameterFloat>(
                        DRIVE_ID, "Drive",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kDrive),

                    std::make_unique<juce::AudioParameterFloat>(
                        VOLUME_ID, "Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kVolume),

                    std::make_unique<juce::AudioParameterInt>(
                        GLOBAL_OCTAVE_ID, "Global Octave",
                        -2, 2, 0), // Range: -2 to +2, default: 0

                    // Analog character parameters
                    std::make_unique<juce::AudioParameterFloat>(
                        DRIFT_ID, "Drift",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: 0 (off)

                    std::make_unique<juce::AudioParameterFloat>(
                        PHASE_RANDOM_ID, "Phase Random",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: 0 (off)

                    std::make_unique<juce::AudioParameterFloat>(
                        UNISON_ID, "Unison",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: 0 (off)

                    // Filter & Saturation Enhancement Parameters
                    std::make_unique<juce::AudioParameterFloat>(
                        FILTER_FEEDBACK_ID, "Filter Feedback",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default: no feedback

                    std::make_unique<juce::AudioParameterChoice>(
                        SATURATION_TYPE_ID, "Saturation Type",
                        juce::StringArray{"Clean", "Warm", "Tube", "Hard", "Acid"},
                        0), // Default: Clean

                    // Delay Parameters
                    std::make_unique<juce::AudioParameterChoice>(
                        DELAY_TIME_ID, "Delay Time",
                        juce::StringArray{"1/16", "1/16.", "1/16T", "1/8", "1/8.", "1/8T", "1/4", "1/4.", "1/4T", "1/2", "1/2.", "1/1"},
                        Defaults::kDelayTime),

                    std::make_unique<juce::AudioParameterFloat>(
                        DELAY_FEEDBACK_ID, "Delay Feedback",
                        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
                        Defaults::kDelayFeedback),

                    std::make_unique<juce::AudioParameterFloat>(
                        DELAY_MIX_ID, "Delay Mix",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kDelayMix),

                    // Cutoff LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        CUTOFF_LFO_RATE_ID, "Cutoff LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        CUTOFF_LFO_WAVE_ID, "Cutoff LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        CUTOFF_LFO_DEPTH_ID, "Cutoff LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Resonance LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        RESONANCE_LFO_RATE_ID, "Resonance LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        RESONANCE_LFO_WAVE_ID, "Resonance LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        RESONANCE_LFO_DEPTH_ID, "Resonance LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // EnvMod LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        ENVMOD_LFO_RATE_ID, "EnvMod LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        ENVMOD_LFO_WAVE_ID, "EnvMod LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        ENVMOD_LFO_DEPTH_ID, "EnvMod LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Decay LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        DECAY_LFO_RATE_ID, "Decay LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        DECAY_LFO_WAVE_ID, "Decay LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        DECAY_LFO_DEPTH_ID, "Decay LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Accent LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        ACCENT_LFO_RATE_ID, "Accent LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        ACCENT_LFO_WAVE_ID, "Accent LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        ACCENT_LFO_DEPTH_ID, "Accent LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Waveform LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        WAVEFORM_LFO_RATE_ID, "Waveform LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        WAVEFORM_LFO_WAVE_ID, "Waveform LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        WAVEFORM_LFO_DEPTH_ID, "Waveform LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // SubOsc LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        SUBOSC_LFO_RATE_ID, "SubOsc LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        SUBOSC_LFO_WAVE_ID, "SubOsc LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        SUBOSC_LFO_DEPTH_ID, "SubOsc LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Drive LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        DRIVE_LFO_RATE_ID, "Drive LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        DRIVE_LFO_WAVE_ID, "Drive LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        DRIVE_LFO_DEPTH_ID, "Drive LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Volume LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        VOLUME_LFO_RATE_ID, "Volume LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        VOLUME_LFO_WAVE_ID, "Volume LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        VOLUME_LFO_DEPTH_ID, "Volume LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // DelayMix LFO
                    std::make_unique<juce::AudioParameterChoice>(
                        DELAYMIX_LFO_RATE_ID, "DelayMix LFO Rate",
                        getLFORateOptions(), Defaults::kLFORate),
                    std::make_unique<juce::AudioParameterChoice>(
                        DELAYMIX_LFO_WAVE_ID, "DelayMix LFO Wave",
                        getLFOWaveformOptions(), Defaults::kLFOWaveform),
                    std::make_unique<juce::AudioParameterFloat>(
                        DELAYMIX_LFO_DEPTH_ID, "DelayMix LFO Depth",
                        juce::NormalisableRange<float>(0.0f, 0.5f, 0.01f),
                        Defaults::kLFODepth),

                    // Arpeggiator parameters
                    std::make_unique<juce::AudioParameterBool>(
                        ARP_ONOFF_ID, "Arp On/Off", true), // Default: enabled
                    std::make_unique<juce::AudioParameterChoice>(
                        ARP_MODE_ID, "Arp Mode",
                        juce::StringArray{"Up", "Down", "Up-Down", "Random", "As Played"}, 0),
                    std::make_unique<juce::AudioParameterChoice>(
                        ARP_RATE_ID, "Arp Rate",
                        juce::StringArray{"1/32", "1/32.", "1/16", "1/16.", "1/16T", "1/8", "1/8.", "1/8T", "1/4", "1/4.", "1/4T", "1/2", "1/2.", "1/1"}, 5), // Default to 1/8
                    std::make_unique<juce::AudioParameterInt>(
                        ARP_OCTAVES_ID, "Arp Octaves",
                        1, 4, 2), // 1-4 octaves, default 2
                    std::make_unique<juce::AudioParameterFloat>(
                        ARP_GATE_ID, "Arp Gate",
                        juce::NormalisableRange<float>(0.1f, 1.0f, 0.01f),
                        0.8f), // Default 80% gate
                    std::make_unique<juce::AudioParameterInt>(
                        ARP_OCTAVE_SHIFT_ID, "Arp Octave Shift",
                        -2, 2, 0), // -2 to +2 octaves, default 0
                    std::make_unique<juce::AudioParameterFloat>(
                        ARP_SWING_ID, "Arp Swing",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f), // Default 0% swing (straight timing)

                    // Sequencer parameters
                    std::make_unique<juce::AudioParameterBool>(
                        SEQ_ENABLED_ID, "Seq Enabled", true), // Default: enabled
                    std::make_unique<juce::AudioParameterChoice>(
                        SEQ_ROOT_ID, "Seq Root",
                        juce::StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 0), // Default to C
                    std::make_unique<juce::AudioParameterChoice>(
                        SEQ_SCALE_ID, "Seq Scale",
                        juce::StringArray{"Major", "Minor", "Dorian", "Phrygian", "Lydian", "Mixolydian", "Aeolian", "Locrian", "Harmonic Minor", "Melodic Minor", "Pentatonic Major", "Pentatonic Minor", "Blues"}, 1), // Default to Minor
                    std::make_unique<juce::AudioParameterInt>(
                        SEQ_STEPS_ID, "Seq Steps",
                        1, 16, 16), // Range 1-16, default 16
                    std::make_unique<juce::AudioParameterChoice>(
                        SEQ_RATE_ID, "Seq Rate",
                        juce::StringArray{"1/32", "1/32.", "1/16", "1/16.", "1/16T", "1/8", "1/8.", "1/8T", "1/4", "1/4.", "1/4T", "1/2", "1/2.", "1/1"}, 2), // Default to 1/16

                    // Sequencer per-step octave (16 steps, range -2 to +2, default 0)
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE1_ID, "Seq Octave 1", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE2_ID, "Seq Octave 2", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE3_ID, "Seq Octave 3", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE4_ID, "Seq Octave 4", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE5_ID, "Seq Octave 5", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE6_ID, "Seq Octave 6", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE7_ID, "Seq Octave 7", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE8_ID, "Seq Octave 8", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE9_ID, "Seq Octave 9", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE10_ID, "Seq Octave 10", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE11_ID, "Seq Octave 11", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE12_ID, "Seq Octave 12", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE13_ID, "Seq Octave 13", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE14_ID, "Seq Octave 14", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE15_ID, "Seq Octave 15", -2, 2, 0),
                    std::make_unique<juce::AudioParameterInt>(SEQ_OCTAVE16_ID, "Seq Octave 16", -2, 2, 0),

                    // Sequencer per-step cutoff modulation (16 steps, range -1.0 to +1.0, default 0.0)
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF1_ID, "Seq Cutoff 1", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF2_ID, "Seq Cutoff 2", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF3_ID, "Seq Cutoff 3", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF4_ID, "Seq Cutoff 4", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF5_ID, "Seq Cutoff 5", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF6_ID, "Seq Cutoff 6", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF7_ID, "Seq Cutoff 7", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF8_ID, "Seq Cutoff 8", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF9_ID, "Seq Cutoff 9", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF10_ID, "Seq Cutoff 10", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF11_ID, "Seq Cutoff 11", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF12_ID, "Seq Cutoff 12", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF13_ID, "Seq Cutoff 13", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF14_ID, "Seq Cutoff 14", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF15_ID, "Seq Cutoff 15", -1.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_CUTOFF16_ID, "Seq Cutoff 16", -1.0f, 1.0f, 0.0f),

                    // Sequencer accent control amounts
                    std::make_unique<juce::AudioParameterFloat>(SEQ_ACCENT_VOL_ID, "Seq Accent Volume", 0.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_ACCENT_CUTOFF_ID, "Seq Accent Cutoff", 0.0f, 1.0f, 0.5f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_ACCENT_RES_ID, "Seq Accent Resonance", 0.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_ACCENT_DECAY_ID, "Seq Accent Decay", 0.0f, 1.0f, 0.0f),
                    std::make_unique<juce::AudioParameterFloat>(SEQ_ACCENT_DRIVE_ID, "Seq Accent Drive", 0.0f, 1.0f, 0.0f),

                    // Progression parameters
                    std::make_unique<juce::AudioParameterBool>(
                        PROG_ENABLED_ID, "Progression Enabled", true), // Default: enabled
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEPS_ID, "Progression Steps", 1, 16, 4), // Default: 4 steps, max: 16
                    std::make_unique<juce::AudioParameterChoice>(
                        PROG_LENGTH_ID, "Progression Length",
                        juce::StringArray{"1/2", "1", "2", "3", "4"}, 1), // Default: 1 bar
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP1_ID, "Progression Step 1", 1, 8, 1), // Default to scale degree 1
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP2_ID, "Progression Step 2", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP3_ID, "Progression Step 3", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP4_ID, "Progression Step 4", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP5_ID, "Progression Step 5", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP6_ID, "Progression Step 6", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP7_ID, "Progression Step 7", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP8_ID, "Progression Step 8", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP9_ID, "Progression Step 9", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP10_ID, "Progression Step 10", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP11_ID, "Progression Step 11", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP12_ID, "Progression Step 12", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP13_ID, "Progression Step 13", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP14_ID, "Progression Step 14", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP15_ID, "Progression Step 15", 1, 8, 1),
                    std::make_unique<juce::AudioParameterInt>(
                        PROG_STEP16_ID, "Progression Step 16", 1, 8, 1),

                    // Global parameters
                    std::make_unique<juce::AudioParameterFloat>(
                        GLOBAL_BPM_ID, "BPM",
                        juce::NormalisableRange<float>(60.0f, 200.0f, 1.0f),
                        120.0f), // Default 120 BPM
                    std::make_unique<juce::AudioParameterFloat>(
                        MASTER_VOLUME_ID, "Master Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.7f), // Default 70%

                    // Drum machine parameters
                    std::make_unique<juce::AudioParameterBool>("drumenable", "Drum Enable", true),
                    std::make_unique<juce::AudioParameterFloat>("drumkickvol", "Kick Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f),
                    std::make_unique<juce::AudioParameterFloat>("drumsnarevol", "Snare Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f),
                    std::make_unique<juce::AudioParameterFloat>("drumchatvol", "Closed Hat Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f),
                    std::make_unique<juce::AudioParameterFloat>("drumohatvol", "Open Hat Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f),
                    std::make_unique<juce::AudioParameterFloat>("drummastervol", "Drum Master Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f),
                    std::make_unique<juce::AudioParameterFloat>("drumsidechainmag", "Sidechain Magnitude",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f),
                    std::make_unique<juce::AudioParameterFloat>("drumsidechainlen", "Sidechain Length",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f)
                })
{
    // Add voices to the synthesizer
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice(new AcidVoice());

    // Add sound
    synth.addSound(new AcidSound());

    // Initialize sequencer pattern with a default melody (C major scale pattern)
    // Pattern: 1-3-5-7-5-3-1-1 (repeated twice) - stored as bitmasks (bit N = degree N active)
    const int defaultDegrees[] = {0, 2, 4, 6, 4, 2, 0, 0, 0, 2, 4, 6, 4, 2, 0, 0};
    for (int i = 0; i < NUM_SEQ_STEPS; ++i)
        sequencerPattern[i] = static_cast<uint8_t>(1 << defaultDegrees[i]);

    // Initialize per-note octave offsets to 0
    for (int step = 0; step < NUM_SEQ_STEPS; ++step)
        for (int degree = 0; degree < NUM_SCALE_DEGREES; ++degree)
            sequencerOctave[step][degree] = 0;

    // Load presets from JSON files
    loadPresetsFromJSON();
    loadDrumSamples();

    // Initialize default drum pattern (4 kicks)
    drumPattern[0][0] = 1;  // Kick on 1
    drumPattern[0][4] = 1;  // Kick on 5
    drumPattern[0][8] = 1;  // Kick on 9
    drumPattern[0][12] = 1; // Kick on 13

    // Load the first synth preset by default
    if (getSynthPresetNames().size() > 0)
        loadPresetFromJSON(0);
}

SnorkelSynthAudioProcessor::~SnorkelSynthAudioProcessor()
{
}

//==============================================================================
const juce::String SnorkelSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SnorkelSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

bool SnorkelSynthAudioProcessor::producesMidi() const
{
    return false;
}

bool SnorkelSynthAudioProcessor::isMidiEffect() const
{
    return false;
}

double SnorkelSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SnorkelSynthAudioProcessor::getNumPrograms()
{
    return getSynthPresetNames().size();
}

int SnorkelSynthAudioProcessor::getCurrentProgram()
{
    return 0; // Could track current preset if needed
}

void SnorkelSynthAudioProcessor::setCurrentProgram(int index)
{
    // Account for the "** User presets **" divider in the preset list
    // The divider appears at position numSystemSynthPresets in the names list
    int actualPresetIndex = index;

    // If divider exists and index is at or past it
    if (numSystemSynthPresets > 0)
    {
        if (index == numSystemSynthPresets)
        {
            // User selected the divider itself - don't load anything
            return;
        }
        else if (index > numSystemSynthPresets)
        {
            // User selected a user preset - adjust index to skip divider
            actualPresetIndex = index - 1;
        }
    }

    loadPresetFromJSON(actualPresetIndex);
}

const juce::String SnorkelSynthAudioProcessor::getProgramName(int index)
{
    juce::StringArray names = getSynthPresetNames();
    if (index >= 0 && index < names.size())
        return names[index];
    return {};
}

void SnorkelSynthAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // Factory presets cannot be renamed
}

//==============================================================================
void SnorkelSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);
    currentSampleRate = sampleRate;

    // Prepare delay line (max 4 seconds delay)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 2;

    delayLine.prepare(spec);
    delayLine.reset();

    // Allocate delay buffer
    delayBuffer.resize(samplesPerBlock * 2, 0.0f);
}

void SnorkelSynthAudioProcessor::releaseResources()
{
}

bool SnorkelSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void SnorkelSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Get BPM from host, or use default for standalone mode
    bool bpmFromHost = false;
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                currentBPM = *positionInfo->getBpm();
                bpmFromHost = true;
            }
            // Enable playback when host transport is playing
            if (positionInfo->getIsPlaying())
            {
                isPlaybackActive = true;
            }
        }
    }

    // Use global BPM parameter if no host BPM available
    if (!bpmFromHost)
    {
        currentBPM = parameters.getRawParameterValue(GLOBAL_BPM_ID)->load();
    }

    // Clear output buffer
    buffer.clear();

    // Track total playback time for bar synchronization
    if (isPlaybackActive)
    {
        totalPlaybackTime += buffer.getNumSamples();
    }

    // Update progression step (must be before arp/sequencer)
    updateProgressionStep(buffer.getNumSamples());

    // Process arpeggiator (modifies MIDI messages)
    processArpeggiator(midiMessages, buffer.getNumSamples());

    // Process sequencer (modifies MIDI messages)
    processSequencer(midiMessages, buffer.getNumSamples());

    // Process drum machine (step tracking and sidechain envelope)
    processDrums(buffer.getNumSamples());

    // Update voice parameters (after sequencer to get correct currentSeqStep for cutoff modulation)
    updateVoiceParameters();

    // Render synthesizer
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply sidechain ducking from drum kick
    if (sidechainEnvelope > 0.001f)
    {
        float duckAmount = std::max(0.0f, 1.0f - sidechainEnvelope); // 0 = full duck, 1 = no duck
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                channelData[sample] *= duckAmount;
            }
        }
    }

    // Mix drum samples into output
    bool drumEnabled = parameters.getRawParameterValue("drumenable")->load() > 0.5f;
    if (drumEnabled)
    {
        float drumMasterVol = parameters.getRawParameterValue("drummastervol")->load();
        const char* laneVolIds[NUM_DRUM_LANES] = { "drumkickvol", "drumsnarevol", "drumchatvol", "drumohatvol" };

        for (int lane = 0; lane < NUM_DRUM_LANES; ++lane)
        {
            if (drumSamplePlaying[lane] && drumSamples[lane].getNumSamples() > 0)
            {
                float laneVol = parameters.getRawParameterValue(laneVolIds[lane])->load();
                float vol = drumMasterVol * laneVol;

                int samplesRemaining = drumSamples[lane].getNumSamples() - drumSamplePositions[lane];
                int samplesToAdd = juce::jmin(buffer.getNumSamples(), samplesRemaining);

                for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
                {
                    int sampleChannel = juce::jmin(channel, drumSamples[lane].getNumChannels() - 1);
                    const float* sampleData = drumSamples[lane].getReadPointer(sampleChannel, drumSamplePositions[lane]);
                    float* outputData = buffer.getWritePointer(channel);

                    for (int i = 0; i < samplesToAdd; ++i)
                    {
                        outputData[i] += sampleData[i] * vol;
                    }
                }

                drumSamplePositions[lane] += samplesToAdd;
                if (drumSamplePositions[lane] >= drumSamples[lane].getNumSamples())
                {
                    drumSamplePlaying[lane] = false;
                }
            }
        }
    }

    // Apply delay effect
    float delayMix = parameters.getRawParameterValue(DELAY_MIX_ID)->load();

    // Update delay mix LFO settings
    updateDelayMixLFO();

    if (delayMix > 0.001f || delayMixLFO.depth > 0.001f)
    {
        int delayTime = static_cast<int>(parameters.getRawParameterValue(DELAY_TIME_ID)->load());
        float delayFeedback = parameters.getRawParameterValue(DELAY_FEEDBACK_ID)->load();

        // Calculate delay time in samples based on tempo
        // Index: 0=1/16, 1=1/16., 2=1/16T, 3=1/8, 4=1/8., 5=1/8T, 6=1/4, 7=1/4., 8=1/4T, 9=1/2, 10=1/2., 11=1/1
        const double divisions[] = {
            4.0,        // 1/16
            4.0/1.5,    // 1/16. (dotted)
            6.0,        // 1/16T (triplet)
            2.0,        // 1/8
            2.0/1.5,    // 1/8. (dotted)
            3.0,        // 1/8T (triplet)
            1.0,        // 1/4
            1.0/1.5,    // 1/4. (dotted)
            1.5,        // 1/4T (triplet)
            0.5,        // 1/2
            0.5/1.5,    // 1/2. (dotted)
            0.25        // 1/1 (whole note)
        };
        double beatsPerSecond = currentBPM / 60.0;
        double notesPerBeat = divisions[delayTime];
        double delayTimeSeconds = 1.0 / (beatsPerSecond * notesPerBeat);
        int delaySamples = static_cast<int>(delayTimeSeconds * currentSampleRate);
        delaySamples = juce::jlimit(1, static_cast<int>(currentSampleRate * 4), delaySamples);

        delayLine.setDelay(static_cast<float>(delaySamples));

        // Process delay for each channel
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            {
                // Get delay mix LFO value and apply modulation
                double lfoValue = getDelayMixLFOValue();
                float modulatedDelayMix = delayMix + static_cast<float>(lfoValue) * delayMixLFO.depth;
                modulatedDelayMix = juce::jlimit(0.0f, 1.0f, modulatedDelayMix);

                // Read delayed sample
                float delayedSample = delayLine.popSample(channel);

                // Mix input with feedback
                float inputSample = channelData[sample];
                float feedbackSample = inputSample + delayedSample * delayFeedback;

                // Push to delay line
                delayLine.pushSample(channel, feedbackSample);

                // Mix dry and wet with LFO-modulated mix
                channelData[sample] = inputSample * (1.0f - modulatedDelayMix) + delayedSample * modulatedDelayMix;

                // Advance LFO phase (only on channel 0 to avoid double-advancing)
                if (channel == 0)
                    advanceDelayMixLFO();
            }
        }
    }

    // Apply master volume to final output
    float masterVolume = parameters.getRawParameterValue(MASTER_VOLUME_ID)->load();
    buffer.applyGain(masterVolume);
}

void SnorkelSynthAudioProcessor::updateVoiceParameters()
{
    // Main parameters
    float cutoff = parameters.getRawParameterValue(CUTOFF_ID)->load();

    // Get base resonance first (needed before accent modulation)
    float resonance = parameters.getRawParameterValue(RESONANCE_ID)->load();
    float drive = parameters.getRawParameterValue(DRIVE_ID)->load();
    float volume = parameters.getRawParameterValue(VOLUME_ID)->load();

    // Apply sequencer accent modulation if sequencer is enabled and playing
    bool seqEnabled = parameters.getRawParameterValue(SEQ_ENABLED_ID)->load() > 0.5f;
    if (seqEnabled && isPlaybackActive)
    {
        float accentValue = getCurrentSeqCutoffMod(); // Range: -1.0 to +1.0

        // Get accent control amounts
        float accentVol = parameters.getRawParameterValue(SEQ_ACCENT_VOL_ID)->load();
        float accentCutoff = parameters.getRawParameterValue(SEQ_ACCENT_CUTOFF_ID)->load();
        float accentRes = parameters.getRawParameterValue(SEQ_ACCENT_RES_ID)->load();
        float accentDecay = parameters.getRawParameterValue(SEQ_ACCENT_DECAY_ID)->load();
        float accentDrive = parameters.getRawParameterValue(SEQ_ACCENT_DRIVE_ID)->load();

        // Apply cutoff modulation (logarithmic scaling)
        float cutoffMod = accentValue * accentCutoff;
        float modulationFactor = std::pow(16.0f, cutoffMod);
        cutoff = juce::jlimit(20.0f, 5000.0f, cutoff * modulationFactor);

        // Apply volume modulation (add up to +6dB on positive accent)
        float volMod = accentValue * accentVol * 0.5f; // ±0.5 range
        volume = juce::jlimit(0.0f, 1.0f, volume + volMod);

        // Apply resonance modulation
        float resMod = accentValue * accentRes * 0.5f;
        resonance = juce::jlimit(0.0f, 1.0f, resonance + resMod);

        // Apply drive modulation
        float driveMod = accentValue * accentDrive * 0.5f;
        drive = juce::jlimit(0.0f, 1.0f, drive + driveMod);

        // Store decay modulation for envelope application below
        seqAccentDecayMod = accentValue * accentDecay;
    }
    else
    {
        seqAccentDecayMod = 0.0f;
    }
    float envMod = parameters.getRawParameterValue(ENV_MOD_ID)->load();
    float accent = parameters.getRawParameterValue(ACCENT_ID)->load();

    // Three oscillators
    float osc1Wave = parameters.getRawParameterValue(OSC1_WAVE_ID)->load();
    int osc1Coarse = static_cast<int>(parameters.getRawParameterValue(OSC1_COARSE_ID)->load());
    float osc1Fine = parameters.getRawParameterValue(OSC1_FINE_ID)->load();
    float osc1Mix = parameters.getRawParameterValue(OSC1_MIX_ID)->load();

    float osc2Wave = parameters.getRawParameterValue(OSC2_WAVE_ID)->load();
    int osc2Coarse = static_cast<int>(parameters.getRawParameterValue(OSC2_COARSE_ID)->load());
    float osc2Fine = parameters.getRawParameterValue(OSC2_FINE_ID)->load();
    float osc2Mix = parameters.getRawParameterValue(OSC2_MIX_ID)->load();

    float osc3Wave = parameters.getRawParameterValue(OSC3_WAVE_ID)->load();
    int osc3Coarse = static_cast<int>(parameters.getRawParameterValue(OSC3_COARSE_ID)->load());
    float osc3Fine = parameters.getRawParameterValue(OSC3_FINE_ID)->load();
    float osc3Mix = parameters.getRawParameterValue(OSC3_MIX_ID)->load();

    float noiseMix = parameters.getRawParameterValue(NOISE_MIX_ID)->load();
    float noiseType = parameters.getRawParameterValue(NOISE_TYPE_ID)->load();
    float noiseDecay = parameters.getRawParameterValue(NOISE_DECAY_ID)->load();

    float filterFeedback = parameters.getRawParameterValue(FILTER_FEEDBACK_ID)->load();
    int saturationType = static_cast<int>(parameters.getRawParameterValue(SATURATION_TYPE_ID)->load());

    // Filter ADSR parameters
    float filterAttack = parameters.getRawParameterValue(FILTER_ATTACK_ID)->load();
    float filterDecay = parameters.getRawParameterValue(FILTER_DECAY_ID)->load();
    float filterSustain = parameters.getRawParameterValue(FILTER_SUSTAIN_ID)->load();
    float filterRelease = parameters.getRawParameterValue(FILTER_RELEASE_ID)->load();

    // Amplitude ADSR parameters
    float ampAttack = parameters.getRawParameterValue(AMP_ATTACK_ID)->load();
    float ampDecay = parameters.getRawParameterValue(AMP_DECAY_ID)->load();
    // Apply accent decay modulation (positive accent = shorter decay for punchier sound)
    if (seqAccentDecayMod != 0.0f)
    {
        float decayMod = seqAccentDecayMod * 0.8f; // Max 80% reduction
        ampDecay = ampDecay * (1.0f - juce::jmax(0.0f, decayMod));
        ampDecay = juce::jmax(0.01f, ampDecay); // Minimum 10ms
    }
    float ampSustain = parameters.getRawParameterValue(AMP_SUSTAIN_ID)->load();
    float ampRelease = parameters.getRawParameterValue(AMP_RELEASE_ID)->load();

    // Dedicated LFO parameters (3 per parameter: rate, waveform, depth)
    int cutoffLFORate = static_cast<int>(parameters.getRawParameterValue(CUTOFF_LFO_RATE_ID)->load());
    int cutoffLFOWave = static_cast<int>(parameters.getRawParameterValue(CUTOFF_LFO_WAVE_ID)->load());
    float cutoffLFODepth = parameters.getRawParameterValue(CUTOFF_LFO_DEPTH_ID)->load();

    int resonanceLFORate = static_cast<int>(parameters.getRawParameterValue(RESONANCE_LFO_RATE_ID)->load());
    int resonanceLFOWave = static_cast<int>(parameters.getRawParameterValue(RESONANCE_LFO_WAVE_ID)->load());
    float resonanceLFODepth = parameters.getRawParameterValue(RESONANCE_LFO_DEPTH_ID)->load();

    int envModLFORate = static_cast<int>(parameters.getRawParameterValue(ENVMOD_LFO_RATE_ID)->load());
    int envModLFOWave = static_cast<int>(parameters.getRawParameterValue(ENVMOD_LFO_WAVE_ID)->load());
    float envModLFODepth = parameters.getRawParameterValue(ENVMOD_LFO_DEPTH_ID)->load();

    int decayLFORate = static_cast<int>(parameters.getRawParameterValue(DECAY_LFO_RATE_ID)->load());
    int decayLFOWave = static_cast<int>(parameters.getRawParameterValue(DECAY_LFO_WAVE_ID)->load());
    float decayLFODepth = parameters.getRawParameterValue(DECAY_LFO_DEPTH_ID)->load();

    int accentLFORate = static_cast<int>(parameters.getRawParameterValue(ACCENT_LFO_RATE_ID)->load());
    int accentLFOWave = static_cast<int>(parameters.getRawParameterValue(ACCENT_LFO_WAVE_ID)->load());
    float accentLFODepth = parameters.getRawParameterValue(ACCENT_LFO_DEPTH_ID)->load();

    int waveformLFORate = static_cast<int>(parameters.getRawParameterValue(WAVEFORM_LFO_RATE_ID)->load());
    int waveformLFOWave = static_cast<int>(parameters.getRawParameterValue(WAVEFORM_LFO_WAVE_ID)->load());
    float waveformLFODepth = parameters.getRawParameterValue(WAVEFORM_LFO_DEPTH_ID)->load();

    int subOscLFORate = static_cast<int>(parameters.getRawParameterValue(SUBOSC_LFO_RATE_ID)->load());
    int subOscLFOWave = static_cast<int>(parameters.getRawParameterValue(SUBOSC_LFO_WAVE_ID)->load());
    float subOscLFODepth = parameters.getRawParameterValue(SUBOSC_LFO_DEPTH_ID)->load();

    int driveLFORate = static_cast<int>(parameters.getRawParameterValue(DRIVE_LFO_RATE_ID)->load());
    int driveLFOWave = static_cast<int>(parameters.getRawParameterValue(DRIVE_LFO_WAVE_ID)->load());
    float driveLFODepth = parameters.getRawParameterValue(DRIVE_LFO_DEPTH_ID)->load();

    int volumeLFORate = static_cast<int>(parameters.getRawParameterValue(VOLUME_LFO_RATE_ID)->load());
    int volumeLFOWave = static_cast<int>(parameters.getRawParameterValue(VOLUME_LFO_WAVE_ID)->load());
    float volumeLFODepth = parameters.getRawParameterValue(VOLUME_LFO_DEPTH_ID)->load();

    int delayMixLFORate = static_cast<int>(parameters.getRawParameterValue(DELAYMIX_LFO_RATE_ID)->load());
    int delayMixLFOWave = static_cast<int>(parameters.getRawParameterValue(DELAYMIX_LFO_WAVE_ID)->load());
    float delayMixLFODepth = parameters.getRawParameterValue(DELAYMIX_LFO_DEPTH_ID)->load();

    int globalOctave = static_cast<int>(parameters.getRawParameterValue(GLOBAL_OCTAVE_ID)->load());

    // Analog character parameters
    float drift = parameters.getRawParameterValue(DRIFT_ID)->load();
    float phaseRandom = parameters.getRawParameterValue(PHASE_RANDOM_ID)->load();
    float unison = parameters.getRawParameterValue(UNISON_ID)->load();

    // Update all voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<AcidVoice*>(synth.getVoice(i)))
        {
            // Set main parameters
            voice->setCutoff(cutoff);
            voice->setResonance(resonance);
            voice->setEnvMod(envMod);
            voice->setAccent(accent);

            // Set three oscillators
            voice->setOscillator1(osc1Wave, osc1Coarse, osc1Fine, osc1Mix);
            voice->setOscillator2(osc2Wave, osc2Coarse, osc2Fine, osc2Mix);
            voice->setOscillator3(osc3Wave, osc3Coarse, osc3Fine, osc3Mix);
            voice->setNoiseMix(noiseMix);
            voice->setNoiseType(noiseType);
            voice->setNoiseDecay(noiseDecay);

            voice->setDrive(drive);
            voice->setVolume(volume);
            voice->setGlobalOctave(globalOctave);
            voice->setBPM(currentBPM);
            voice->setFilterFeedback(filterFeedback);
            voice->setSaturationType(saturationType);

            // Set analog character parameters
            voice->setDrift(drift);
            voice->setPhaseRandom(phaseRandom);
            voice->setUnison(unison);

            // Set ADSR parameters
            voice->setFilterADSR(filterAttack, filterDecay, filterSustain, filterRelease);
            voice->setAmpADSR(ampAttack, ampDecay, ampSustain, ampRelease);

            // Set all 10 dedicated LFOs
            voice->setCutoffLFO(cutoffLFORate, cutoffLFOWave, cutoffLFODepth);
            voice->setResonanceLFO(resonanceLFORate, resonanceLFOWave, resonanceLFODepth);
            voice->setEnvModLFO(envModLFORate, envModLFOWave, envModLFODepth);
            voice->setDecayLFO(decayLFORate, decayLFOWave, decayLFODepth);
            voice->setAccentLFO(accentLFORate, accentLFOWave, accentLFODepth);
            voice->setWaveformLFO(waveformLFORate, waveformLFOWave, waveformLFODepth);
            voice->setSubOscLFO(subOscLFORate, subOscLFOWave, subOscLFODepth);
            voice->setDriveLFO(driveLFORate, driveLFOWave, driveLFODepth);
            voice->setVolumeLFO(volumeLFORate, volumeLFOWave, volumeLFODepth);
            voice->setDelayMixLFO(delayMixLFORate, delayMixLFOWave, delayMixLFODepth);
        }
    }
}

void SnorkelSynthAudioProcessor::loadPresetFromJSON(int presetIndex)
{
    if (!synthPresetsJSON.isObject())
        return;

    auto* obj = synthPresetsJSON.getDynamicObject();
    if (obj == nullptr)
        return;

    const juce::Array<juce::var>* presetsArray = obj->getProperty("presets").getArray();
    if (presetsArray == nullptr || presetIndex < 0 || presetIndex >= presetsArray->size())
        return;

    auto* presetObj = (*presetsArray)[presetIndex].getDynamicObject();
    if (presetObj == nullptr)
        return;

    // Set all parameters from JSON preset
    parameters.getParameter(CUTOFF_ID)->setValueNotifyingHost(
        parameters.getParameterRange(CUTOFF_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("cutoff"))));

    parameters.getParameter(RESONANCE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(RESONANCE_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("resonance"))));

    parameters.getParameter(ENV_MOD_ID)->setValueNotifyingHost(
        parameters.getParameterRange(ENV_MOD_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("envMod"))));

    parameters.getParameter(ACCENT_ID)->setValueNotifyingHost(
        parameters.getParameterRange(ACCENT_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("accent"))));

    // Load oscillator parameters - support both new and old preset formats
    // New presets have "osc1Wave", "osc1Coarse", etc.
    // Old presets have "waveform" (maps to osc1Wave) and "subOsc" (maps to osc3Mix)

    // Oscillator 1
    float osc1Wave = presetObj->hasProperty("osc1Wave") ?
        static_cast<float>(presetObj->getProperty("osc1Wave")) :
        (presetObj->hasProperty("waveform") ?
            static_cast<float>(presetObj->getProperty("waveform")) : 0.5f);
    parameters.getParameter(OSC1_WAVE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC1_WAVE_ID).convertTo0to1(osc1Wave));

    int osc1Coarse = presetObj->hasProperty("osc1Coarse") ?
        static_cast<int>(presetObj->getProperty("osc1Coarse")) : 0;
    parameters.getParameter(OSC1_COARSE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC1_COARSE_ID).convertTo0to1(osc1Coarse));

    float osc1Fine = presetObj->hasProperty("osc1Fine") ?
        static_cast<float>(presetObj->getProperty("osc1Fine")) : 0.0f;
    parameters.getParameter(OSC1_FINE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC1_FINE_ID).convertTo0to1(osc1Fine));

    float osc1Mix = presetObj->hasProperty("osc1Mix") ?
        static_cast<float>(presetObj->getProperty("osc1Mix")) : 0.7f;
    parameters.getParameter(OSC1_MIX_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC1_MIX_ID).convertTo0to1(osc1Mix));

    // Oscillator 2
    float osc2Wave = presetObj->hasProperty("osc2Wave") ?
        static_cast<float>(presetObj->getProperty("osc2Wave")) : 0.5f;
    parameters.getParameter(OSC2_WAVE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC2_WAVE_ID).convertTo0to1(osc2Wave));

    int osc2Coarse = presetObj->hasProperty("osc2Coarse") ?
        static_cast<int>(presetObj->getProperty("osc2Coarse")) : 0;
    parameters.getParameter(OSC2_COARSE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC2_COARSE_ID).convertTo0to1(osc2Coarse));

    float osc2Fine = presetObj->hasProperty("osc2Fine") ?
        static_cast<float>(presetObj->getProperty("osc2Fine")) : 0.0f;
    parameters.getParameter(OSC2_FINE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC2_FINE_ID).convertTo0to1(osc2Fine));

    float osc2Mix = presetObj->hasProperty("osc2Mix") ?
        static_cast<float>(presetObj->getProperty("osc2Mix")) : 0.0f;
    parameters.getParameter(OSC2_MIX_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC2_MIX_ID).convertTo0to1(osc2Mix));

    // Oscillator 3
    float osc3Wave = presetObj->hasProperty("osc3Wave") ?
        static_cast<float>(presetObj->getProperty("osc3Wave")) : 0.0f;
    parameters.getParameter(OSC3_WAVE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC3_WAVE_ID).convertTo0to1(osc3Wave));

    int osc3Coarse = presetObj->hasProperty("osc3Coarse") ?
        static_cast<int>(presetObj->getProperty("osc3Coarse")) : -12;
    parameters.getParameter(OSC3_COARSE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC3_COARSE_ID).convertTo0to1(osc3Coarse));

    float osc3Fine = presetObj->hasProperty("osc3Fine") ?
        static_cast<float>(presetObj->getProperty("osc3Fine")) : 0.0f;
    parameters.getParameter(OSC3_FINE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC3_FINE_ID).convertTo0to1(osc3Fine));

    float osc3Mix = presetObj->hasProperty("osc3Mix") ?
        static_cast<float>(presetObj->getProperty("osc3Mix")) :
        (presetObj->hasProperty("subOsc") ?
            static_cast<float>(presetObj->getProperty("subOsc")) : 0.5f);
    parameters.getParameter(OSC3_MIX_ID)->setValueNotifyingHost(
        parameters.getParameterRange(OSC3_MIX_ID).convertTo0to1(osc3Mix));

    // Noise oscillator
    if (presetObj->hasProperty("noiseType"))
    {
        float noiseType = static_cast<float>(presetObj->getProperty("noiseType"));
        parameters.getParameter(NOISE_TYPE_ID)->setValueNotifyingHost(
            parameters.getParameterRange(NOISE_TYPE_ID).convertTo0to1(noiseType));
    }

    if (presetObj->hasProperty("noiseDecay"))
    {
        float noiseDecay = static_cast<float>(presetObj->getProperty("noiseDecay"));
        parameters.getParameter(NOISE_DECAY_ID)->setValueNotifyingHost(
            parameters.getParameterRange(NOISE_DECAY_ID).convertTo0to1(noiseDecay));
    }

    if (presetObj->hasProperty("noiseMix"))
    {
        float noiseMix = static_cast<float>(presetObj->getProperty("noiseMix"));
        parameters.getParameter(NOISE_MIX_ID)->setValueNotifyingHost(
            parameters.getParameterRange(NOISE_MIX_ID).convertTo0to1(noiseMix));
    }

    parameters.getParameter(DRIVE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DRIVE_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("drive"))));

    parameters.getParameter(VOLUME_ID)->setValueNotifyingHost(
        parameters.getParameterRange(VOLUME_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("volume"))));

    // Global octave
    if (presetObj->hasProperty("globalOctave"))
    {
        int globalOctave = static_cast<int>(presetObj->getProperty("globalOctave"));
        parameters.getParameter(GLOBAL_OCTAVE_ID)->setValueNotifyingHost(
            parameters.getParameterRange(GLOBAL_OCTAVE_ID).convertTo0to1(static_cast<float>(globalOctave)));
    }

    // Analog character parameters
    if (presetObj->hasProperty("drift"))
    {
        float drift = static_cast<float>(presetObj->getProperty("drift"));
        parameters.getParameter(DRIFT_ID)->setValueNotifyingHost(
            parameters.getParameterRange(DRIFT_ID).convertTo0to1(drift));
    }

    if (presetObj->hasProperty("phaseRandom"))
    {
        float phaseRandom = static_cast<float>(presetObj->getProperty("phaseRandom"));
        parameters.getParameter(PHASE_RANDOM_ID)->setValueNotifyingHost(
            parameters.getParameterRange(PHASE_RANDOM_ID).convertTo0to1(phaseRandom));
    }

    if (presetObj->hasProperty("unison"))
    {
        float unison = static_cast<float>(presetObj->getProperty("unison"));
        parameters.getParameter(UNISON_ID)->setValueNotifyingHost(
            parameters.getParameterRange(UNISON_ID).convertTo0to1(unison));
    }

    // Amplitude ADSR
    if (presetObj->hasProperty("ampAttack"))
    {
        float ampAttack = static_cast<float>(presetObj->getProperty("ampAttack"));
        parameters.getParameter(AMP_ATTACK_ID)->setValueNotifyingHost(
            parameters.getParameterRange(AMP_ATTACK_ID).convertTo0to1(ampAttack));
    }

    if (presetObj->hasProperty("ampDecay"))
    {
        float ampDecay = static_cast<float>(presetObj->getProperty("ampDecay"));
        parameters.getParameter(AMP_DECAY_ID)->setValueNotifyingHost(
            parameters.getParameterRange(AMP_DECAY_ID).convertTo0to1(ampDecay));
    }

    if (presetObj->hasProperty("ampSustain"))
    {
        float ampSustain = static_cast<float>(presetObj->getProperty("ampSustain"));
        parameters.getParameter(AMP_SUSTAIN_ID)->setValueNotifyingHost(
            parameters.getParameterRange(AMP_SUSTAIN_ID).convertTo0to1(ampSustain));
    }

    if (presetObj->hasProperty("ampRelease"))
    {
        float ampRelease = static_cast<float>(presetObj->getProperty("ampRelease"));
        parameters.getParameter(AMP_RELEASE_ID)->setValueNotifyingHost(
            parameters.getParameterRange(AMP_RELEASE_ID).convertTo0to1(ampRelease));
    }

    // Filter feedback
    if (presetObj->hasProperty("filterFeedback"))
    {
        float filterFeedback = static_cast<float>(presetObj->getProperty("filterFeedback"));
        parameters.getParameter(FILTER_FEEDBACK_ID)->setValueNotifyingHost(
            parameters.getParameterRange(FILTER_FEEDBACK_ID).convertTo0to1(filterFeedback));
    }

    // Saturation type (Choice parameter)
    if (presetObj->hasProperty("saturationType"))
    {
        int saturationType = static_cast<int>(presetObj->getProperty("saturationType"));
        parameters.getParameter(SATURATION_TYPE_ID)->setValueNotifyingHost(
            parameters.getParameterRange(SATURATION_TYPE_ID).convertTo0to1(static_cast<float>(saturationType)));
    }

    // Filter ADSR
    if (presetObj->hasProperty("filterAttack"))
    {
        float filterAttack = static_cast<float>(presetObj->getProperty("filterAttack"));
        parameters.getParameter(FILTER_ATTACK_ID)->setValueNotifyingHost(
            parameters.getParameterRange(FILTER_ATTACK_ID).convertTo0to1(filterAttack));
    }

    if (presetObj->hasProperty("filterDecay"))
    {
        float filterDecay = static_cast<float>(presetObj->getProperty("filterDecay"));
        parameters.getParameter(FILTER_DECAY_ID)->setValueNotifyingHost(
            parameters.getParameterRange(FILTER_DECAY_ID).convertTo0to1(filterDecay));
    }

    if (presetObj->hasProperty("filterSustain"))
    {
        float filterSustain = static_cast<float>(presetObj->getProperty("filterSustain"));
        parameters.getParameter(FILTER_SUSTAIN_ID)->setValueNotifyingHost(
            parameters.getParameterRange(FILTER_SUSTAIN_ID).convertTo0to1(filterSustain));
    }

    if (presetObj->hasProperty("filterRelease"))
    {
        float filterRelease = static_cast<float>(presetObj->getProperty("filterRelease"));
        parameters.getParameter(FILTER_RELEASE_ID)->setValueNotifyingHost(
            parameters.getParameterRange(FILTER_RELEASE_ID).convertTo0to1(filterRelease));
    }

    // Delay Time is a Choice parameter - need to normalize index (0-11) to 0-1 range
    int delayTimeIndex = static_cast<int>(presetObj->getProperty("delayTime"));
    parameters.getParameter(DELAY_TIME_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DELAY_TIME_ID).convertTo0to1(static_cast<float>(delayTimeIndex)));

    parameters.getParameter(DELAY_FEEDBACK_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DELAY_FEEDBACK_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("delayFeedback"))));

    parameters.getParameter(DELAY_MIX_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DELAY_MIX_ID).convertTo0to1(static_cast<float>(presetObj->getProperty("delayMix"))));
}

//==============================================================================
// JSON Preset Management

juce::File SnorkelSynthAudioProcessor::getDataDirectory() const
{
    // Get the plugin binary location and look for data folder relative to it
    juce::File pluginDir = juce::File::getSpecialLocation(juce::File::currentApplicationFile).getParentDirectory();
    juce::File dataDir = pluginDir.getChildFile("data");

    // If not found, try looking in the parent directory (for development builds)
    if (!dataDir.exists())
        dataDir = pluginDir.getParentDirectory().getChildFile("data");

    // If still not found, try current working directory as fallback
    if (!dataDir.exists())
        dataDir = juce::File::getCurrentWorkingDirectory().getChildFile("data");

    return dataDir;
}

juce::String SnorkelSynthAudioProcessor::formatJSON(const juce::var& json, int indentLevel) const
{
    juce::String indent = juce::String::repeatedString(" ", indentLevel * 2);
    juce::String nextIndent = juce::String::repeatedString(" ", (indentLevel + 1) * 2);

    if (json.isObject())
    {
        auto* obj = json.getDynamicObject();
        if (obj == nullptr)
            return "{}";

        juce::String result = "{\n";
        auto& properties = obj->getProperties();
        int count = 0;
        int total = properties.size();

        for (auto& prop : properties)
        {
            result += nextIndent + "\"" + prop.name.toString() + "\": ";
            result += formatJSON(prop.value, indentLevel + 1);
            if (++count < total)
                result += ",";
            result += "\n";
        }

        result += indent + "}";
        return result;
    }
    else if (json.isArray())
    {
        auto* arr = json.getArray();
        if (arr == nullptr || arr->isEmpty())
            return "[]";

        // Check if array contains simple values (numbers) - keep on one line
        bool simpleArray = true;
        for (auto& item : *arr)
        {
            if (item.isObject() || item.isArray())
            {
                simpleArray = false;
                break;
            }
        }

        if (simpleArray)
        {
            juce::String result = "[";
            for (int i = 0; i < arr->size(); ++i)
            {
                result += formatJSON((*arr)[i], indentLevel);
                if (i < arr->size() - 1)
                    result += ", ";
            }
            result += "]";
            return result;
        }
        else
        {
            juce::String result = "[\n";
            for (int i = 0; i < arr->size(); ++i)
            {
                result += nextIndent + formatJSON((*arr)[i], indentLevel + 1);
                if (i < arr->size() - 1)
                    result += ",";
                result += "\n";
            }
            result += indent + "]";
            return result;
        }
    }
    else if (json.isString())
    {
        return "\"" + json.toString() + "\"";
    }
    else
    {
        return json.toString();
    }
}

void SnorkelSynthAudioProcessor::loadPresetsFromJSON()
{
    juce::File dataDir = getDataDirectory();

    // Log loading attempt
    juce::File logFile = dataDir.getChildFile("preset_load_log.txt");
    juce::String logMessage = juce::Time::getCurrentTime().toString(true, true) + "\n";
    logMessage += "Loading presets from: " + dataDir.getFullPathName() + "\n";

    // Load system synth presets (read-only)
    juce::Array<juce::var> combinedPresets;
    numSystemSynthPresets = 0;

    juce::File systemPresetFile = dataDir.getChildFile("synth_presets_system.json");
    logMessage += "System preset file exists: " + juce::String(systemPresetFile.existsAsFile() ? "yes" : "no") + "\n";
    logMessage += "System preset file path: " + systemPresetFile.getFullPathName() + "\n";

    if (systemPresetFile.existsAsFile())
    {
        juce::String jsonText = systemPresetFile.loadFileAsString();
        logMessage += "System JSON length: " + juce::String(jsonText.length()) + "\n";
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            auto* obj = result.getDynamicObject();
            if (obj != nullptr)
            {
                const juce::Array<juce::var>* arr = obj->getProperty("presets").getArray();
                if (arr != nullptr)
                {
                    combinedPresets.addArray(*arr);
                    numSystemSynthPresets = arr->size();
                    logMessage += "System presets loaded: " + juce::String(numSystemSynthPresets) + "\n";
                }
            }
        }
        else
        {
            logMessage += "System JSON parse failed\n";
        }
    }

    // Load user synth presets (writable)
    juce::File userPresetFile = dataDir.getChildFile("synth_presets_user.json");
    logMessage += "User preset file exists: " + juce::String(userPresetFile.existsAsFile() ? "yes" : "no") + "\n";
    logMessage += "User preset file path: " + userPresetFile.getFullPathName() + "\n";

    if (userPresetFile.existsAsFile())
    {
        juce::String jsonText = userPresetFile.loadFileAsString();
        logMessage += "User JSON length: " + juce::String(jsonText.length()) + "\n";
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            auto* obj = result.getDynamicObject();
            if (obj != nullptr)
            {
                const juce::Array<juce::var>* arr = obj->getProperty("presets").getArray();
                if (arr != nullptr)
                {
                    combinedPresets.addArray(*arr);
                    logMessage += "User presets loaded: " + juce::String(arr->size()) + "\n";
                }
            }
        }
        else
        {
            logMessage += "User JSON parse failed\n";
        }
    }

    // Create combined JSON structure
    synthPresetsJSON = juce::var(new juce::DynamicObject());
    synthPresetsJSON.getDynamicObject()->setProperty("presets", combinedPresets);
    logMessage += "Total combined presets: " + juce::String(combinedPresets.size()) + "\n";

    // Load sequencer presets from both system and user files
    juce::Array<juce::var> combinedSequencerPresets;
    numSystemSequencerPresets = 0;

    // Load system sequencer presets (read-only)
    juce::File systemSeqPresetFile = dataDir.getChildFile("sequencer_presets_system.json");
    logMessage += "System sequencer preset file exists: " + juce::String(systemSeqPresetFile.existsAsFile() ? "yes" : "no") + "\n";

    if (systemSeqPresetFile.existsAsFile())
    {
        juce::String jsonText = systemSeqPresetFile.loadFileAsString();
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            auto* obj = result.getDynamicObject();
            if (obj != nullptr)
            {
                const juce::Array<juce::var>* arr = obj->getProperty("presets").getArray();
                if (arr != nullptr)
                {
                    combinedSequencerPresets.addArray(*arr);
                    numSystemSequencerPresets = arr->size();
                    logMessage += "System sequencer presets loaded: " + juce::String(arr->size()) + "\n";
                }
            }
        }
    }

    // Load user sequencer presets (writable)
    juce::File userSeqPresetFile = dataDir.getChildFile("sequencer_presets_user.json");
    logMessage += "User sequencer preset file exists: " + juce::String(userSeqPresetFile.existsAsFile() ? "yes" : "no") + "\n";

    if (userSeqPresetFile.existsAsFile())
    {
        juce::String jsonText = userSeqPresetFile.loadFileAsString();
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            auto* obj = result.getDynamicObject();
            if (obj != nullptr)
            {
                const juce::Array<juce::var>* arr = obj->getProperty("presets").getArray();
                if (arr != nullptr)
                {
                    combinedSequencerPresets.addArray(*arr);
                    logMessage += "User sequencer presets loaded: " + juce::String(arr->size()) + "\n";
                }
            }
        }
    }

    // Create combined sequencer JSON structure
    sequencerPresetsJSON = juce::var(new juce::DynamicObject());
    sequencerPresetsJSON.getDynamicObject()->setProperty("presets", combinedSequencerPresets);
    logMessage += "Total combined sequencer presets: " + juce::String(combinedSequencerPresets.size()) + "\n";

    // Load randomization config
    juce::File randomConfigFile = dataDir.getChildFile("randomization_config.json");
    logMessage += "Randomization config file exists: " + juce::String(randomConfigFile.existsAsFile() ? "yes" : "no") + "\n";

    if (randomConfigFile.existsAsFile())
    {
        juce::String jsonText = randomConfigFile.loadFileAsString();
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            randomizationConfigJSON = result;
            logMessage += "Randomization config loaded\n";
        }
    }

    logMessage += "\n";
    logFile.appendText(logMessage);
}

void SnorkelSynthAudioProcessor::saveSynthPresetToJSON(const juce::String& presetName)
{
    // Create preset object with ALL Osc and Filter tab parameters
    juce::var preset = new juce::DynamicObject();
    auto* presetObj = preset.getDynamicObject();
    presetObj->setProperty("name", presetName);
    presetObj->setProperty("description", "User preset"); // Add description for consistency

    // Filter parameters
    presetObj->setProperty("cutoff", parameters.getRawParameterValue(CUTOFF_ID)->load());
    presetObj->setProperty("resonance", parameters.getRawParameterValue(RESONANCE_ID)->load());
    presetObj->setProperty("envMod", parameters.getRawParameterValue(ENV_MOD_ID)->load());
    presetObj->setProperty("accent", parameters.getRawParameterValue(ACCENT_ID)->load());
    presetObj->setProperty("filterFeedback", parameters.getRawParameterValue(FILTER_FEEDBACK_ID)->load());
    presetObj->setProperty("saturationType", static_cast<int>(parameters.getRawParameterValue(SATURATION_TYPE_ID)->load()));

    // Oscillator 1-3 parameters
    presetObj->setProperty("osc1Wave", parameters.getRawParameterValue(OSC1_WAVE_ID)->load());
    presetObj->setProperty("osc1Coarse", static_cast<int>(parameters.getRawParameterValue(OSC1_COARSE_ID)->load()));
    presetObj->setProperty("osc1Fine", parameters.getRawParameterValue(OSC1_FINE_ID)->load());
    presetObj->setProperty("osc1Mix", parameters.getRawParameterValue(OSC1_MIX_ID)->load());

    presetObj->setProperty("osc2Wave", parameters.getRawParameterValue(OSC2_WAVE_ID)->load());
    presetObj->setProperty("osc2Coarse", static_cast<int>(parameters.getRawParameterValue(OSC2_COARSE_ID)->load()));
    presetObj->setProperty("osc2Fine", parameters.getRawParameterValue(OSC2_FINE_ID)->load());
    presetObj->setProperty("osc2Mix", parameters.getRawParameterValue(OSC2_MIX_ID)->load());

    presetObj->setProperty("osc3Wave", parameters.getRawParameterValue(OSC3_WAVE_ID)->load());
    presetObj->setProperty("osc3Coarse", static_cast<int>(parameters.getRawParameterValue(OSC3_COARSE_ID)->load()));
    presetObj->setProperty("osc3Fine", parameters.getRawParameterValue(OSC3_FINE_ID)->load());
    presetObj->setProperty("osc3Mix", parameters.getRawParameterValue(OSC3_MIX_ID)->load());

    // Noise oscillator
    presetObj->setProperty("noiseType", static_cast<int>(parameters.getRawParameterValue(NOISE_TYPE_ID)->load()));
    presetObj->setProperty("noiseDecay", parameters.getRawParameterValue(NOISE_DECAY_ID)->load());
    presetObj->setProperty("noiseMix", parameters.getRawParameterValue(NOISE_MIX_ID)->load());

    // Global controls
    presetObj->setProperty("drive", parameters.getRawParameterValue(DRIVE_ID)->load());
    presetObj->setProperty("volume", parameters.getRawParameterValue(VOLUME_ID)->load());
    presetObj->setProperty("globalOctave", static_cast<int>(parameters.getRawParameterValue(GLOBAL_OCTAVE_ID)->load()));

    // Analog character
    presetObj->setProperty("drift", parameters.getRawParameterValue(DRIFT_ID)->load());
    presetObj->setProperty("phaseRandom", parameters.getRawParameterValue(PHASE_RANDOM_ID)->load());
    presetObj->setProperty("unison", parameters.getRawParameterValue(UNISON_ID)->load());

    // Amp ADSR
    presetObj->setProperty("ampAttack", parameters.getRawParameterValue(AMP_ATTACK_ID)->load());
    presetObj->setProperty("ampDecay", parameters.getRawParameterValue(AMP_DECAY_ID)->load());
    presetObj->setProperty("ampSustain", parameters.getRawParameterValue(AMP_SUSTAIN_ID)->load());
    presetObj->setProperty("ampRelease", parameters.getRawParameterValue(AMP_RELEASE_ID)->load());

    // Filter ADSR
    presetObj->setProperty("filterAttack", parameters.getRawParameterValue(FILTER_ATTACK_ID)->load());
    presetObj->setProperty("filterDecay", parameters.getRawParameterValue(FILTER_DECAY_ID)->load());
    presetObj->setProperty("filterSustain", parameters.getRawParameterValue(FILTER_SUSTAIN_ID)->load());
    presetObj->setProperty("filterRelease", parameters.getRawParameterValue(FILTER_RELEASE_ID)->load());

    // Delay
    presetObj->setProperty("delayTime", static_cast<int>(parameters.getRawParameterValue(DELAY_TIME_ID)->load()));
    presetObj->setProperty("delayFeedback", parameters.getRawParameterValue(DELAY_FEEDBACK_ID)->load());
    presetObj->setProperty("delayMix", parameters.getRawParameterValue(DELAY_MIX_ID)->load());

    // FIRST: Update the in-memory combined preset list immediately
    if (synthPresetsJSON.isObject())
    {
        auto* combinedObj = synthPresetsJSON.getDynamicObject();
        if (combinedObj != nullptr)
        {
            juce::Array<juce::var>* combinedArray = combinedObj->getProperty("presets").getArray();
            if (combinedArray != nullptr)
            {
                combinedArray->add(preset);
            }
        }
    }

    // SECOND: Load existing user presets from file
    juce::File dataDir = getDataDirectory();
    dataDir.createDirectory();
    juce::File userPresetFile = dataDir.getChildFile("synth_presets_user.json");

    juce::Array<juce::var> userPresetsArray;

    if (userPresetFile.existsAsFile())
    {
        juce::String jsonText = userPresetFile.loadFileAsString();
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            auto* obj = result.getDynamicObject();
            if (obj != nullptr)
            {
                const juce::Array<juce::var>* arr = obj->getProperty("presets").getArray();
                if (arr != nullptr)
                {
                    userPresetsArray = *arr;
                }
            }
        }
    }

    // Add new preset
    userPresetsArray.add(preset);

    // Build user presets JSON structure
    juce::var userPresetsRoot = new juce::DynamicObject();
    juce::var presetsArrayVar(userPresetsArray);
    userPresetsRoot.getDynamicObject()->setProperty("presets", presetsArrayVar);

    // Use JUCE's built-in JSON serializer
    juce::String jsonOutput = juce::JSON::toString(userPresetsRoot, true);
    userPresetFile.replaceWithText(jsonOutput);
}

void SnorkelSynthAudioProcessor::saveSequencerPresetToJSON(const juce::String& presetName)
{
    // Create preset object
    juce::var preset = new juce::DynamicObject();
    auto* presetObj = preset.getDynamicObject();
    presetObj->setProperty("name", presetName);
    presetObj->setProperty("description", "User preset");
    presetObj->setProperty("version", 2); // New format version

    // Save pattern as bitmasks (multiple notes per step)
    juce::Array<juce::var> pattern;
    for (int i = 0; i < 16; ++i)
        pattern.add(static_cast<int>(sequencerPattern[i])); // uint8_t bitmask
    presetObj->setProperty("pattern", pattern);

    // Save per-note octave values (2D array: 16 steps x 8 degrees)
    juce::Array<juce::var> octaves;
    for (int step = 0; step < 16; ++step)
    {
        juce::Array<juce::var> stepOctaves;
        for (int degree = 0; degree < 8; ++degree)
            stepOctaves.add(static_cast<int>(sequencerOctave[step][degree]));
        octaves.add(juce::var(stepOctaves));
    }
    presetObj->setProperty("octave", octaves);

    // Save accent values per step (seqcutoff1-16)
    const char* accentParamIds[] = {
        SEQ_CUTOFF1_ID, SEQ_CUTOFF2_ID, SEQ_CUTOFF3_ID, SEQ_CUTOFF4_ID,
        SEQ_CUTOFF5_ID, SEQ_CUTOFF6_ID, SEQ_CUTOFF7_ID, SEQ_CUTOFF8_ID,
        SEQ_CUTOFF9_ID, SEQ_CUTOFF10_ID, SEQ_CUTOFF11_ID, SEQ_CUTOFF12_ID,
        SEQ_CUTOFF13_ID, SEQ_CUTOFF14_ID, SEQ_CUTOFF15_ID, SEQ_CUTOFF16_ID
    };
    juce::Array<juce::var> accents;
    for (int i = 0; i < 16; ++i)
        accents.add(parameters.getRawParameterValue(accentParamIds[i])->load());
    presetObj->setProperty("accents", accents);

    // Save accent dial settings
    presetObj->setProperty("accentVol", parameters.getRawParameterValue(SEQ_ACCENT_VOL_ID)->load());
    presetObj->setProperty("accentCutoff", parameters.getRawParameterValue(SEQ_ACCENT_CUTOFF_ID)->load());
    presetObj->setProperty("accentRes", parameters.getRawParameterValue(SEQ_ACCENT_RES_ID)->load());
    presetObj->setProperty("accentDecay", parameters.getRawParameterValue(SEQ_ACCENT_DECAY_ID)->load());
    presetObj->setProperty("accentDrive", parameters.getRawParameterValue(SEQ_ACCENT_DRIVE_ID)->load());

    // FIRST: Update the in-memory combined preset list immediately
    if (sequencerPresetsJSON.isObject())
    {
        auto* combinedObj = sequencerPresetsJSON.getDynamicObject();
        if (combinedObj != nullptr)
        {
            juce::Array<juce::var>* combinedArray = combinedObj->getProperty("presets").getArray();
            if (combinedArray != nullptr)
            {
                combinedArray->add(preset);
            }
        }
    }

    // SECOND: Load existing user presets from file
    juce::File dataDir = getDataDirectory();
    dataDir.createDirectory();
    juce::File userSeqPresetFile = dataDir.getChildFile("sequencer_presets_user.json");

    juce::Array<juce::var> userPresetsArray;

    if (userSeqPresetFile.existsAsFile())
    {
        juce::String jsonText = userSeqPresetFile.loadFileAsString();
        auto result = juce::JSON::parse(jsonText);
        if (result.isObject())
        {
            auto* obj = result.getDynamicObject();
            if (obj != nullptr)
            {
                const juce::Array<juce::var>* arr = obj->getProperty("presets").getArray();
                if (arr != nullptr)
                {
                    userPresetsArray = *arr;
                }
            }
        }
    }

    // Add new preset
    userPresetsArray.add(preset);

    // Build user presets JSON structure
    juce::var userPresetsRoot = new juce::DynamicObject();
    juce::var presetsArrayVar(userPresetsArray);
    userPresetsRoot.getDynamicObject()->setProperty("presets", presetsArrayVar);

    // Use JUCE's built-in JSON serializer
    juce::String jsonOutput = juce::JSON::toString(userPresetsRoot, true);
    userSeqPresetFile.replaceWithText(jsonOutput);
}

juce::StringArray SnorkelSynthAudioProcessor::getSynthPresetNames() const
{
    juce::StringArray names;

    if (synthPresetsJSON.isObject())
    {
        auto* obj = synthPresetsJSON.getDynamicObject();
        if (obj != nullptr)
        {
            const juce::Array<juce::var>* presetsArray = obj->getProperty("presets").getArray();
            if (presetsArray != nullptr)
            {
                for (int i = 0; i < presetsArray->size(); ++i)
                {
                    // Add divider before user presets
                    if (i == numSystemSynthPresets && numSystemSynthPresets > 0 && i < presetsArray->size())
                    {
                        names.add("** User presets **");
                    }

                    if (auto* presetObj = (*presetsArray)[i].getDynamicObject())
                        names.add(presetObj->getProperty("name").toString());
                }
            }
        }
    }

    return names;
}

juce::StringArray SnorkelSynthAudioProcessor::getSequencerPresetNames() const
{
    juce::StringArray names;

    if (sequencerPresetsJSON.isObject())
    {
        auto* obj = sequencerPresetsJSON.getDynamicObject();
        if (obj != nullptr)
        {
            const juce::Array<juce::var>* presetsArray = obj->getProperty("presets").getArray();
            if (presetsArray != nullptr)
            {
                for (int i = 0; i < presetsArray->size(); ++i)
                {
                    // Add divider before user presets
                    if (i == numSystemSequencerPresets && numSystemSequencerPresets > 0 && i < presetsArray->size())
                    {
                        names.add("** User presets **");
                    }

                    if (auto* presetObj = (*presetsArray)[i].getDynamicObject())
                        names.add(presetObj->getProperty("name").toString());
                }
            }
        }
    }

    return names;
}

//==============================================================================
bool SnorkelSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* SnorkelSynthAudioProcessor::createEditor()
{
    return new SnorkelSynthAudioProcessorEditor(*this);
}

//==============================================================================
void SnorkelSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SnorkelSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // Disabled: Always start with fresh Init preset, don't restore previous state
    // This ensures the plugin/standalone always starts with known default values

    // std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    // if (xmlState.get() != nullptr)
    //     if (xmlState->hasTagName(parameters.state.getType()))
    //         parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// Arpeggiator Implementation

void SnorkelSynthAudioProcessor::processArpeggiator(juce::MidiBuffer& midiMessages, int numSamples)
{
    bool arpEnabled = parameters.getRawParameterValue(ARP_ONOFF_ID)->load() > 0.5f;

    // If arpeggiator is disabled, clear state and pass through MIDI
    if (!arpEnabled)
    {
        heldNotes.clear();
        currentArpNote = 0;
        arpStepTime = 0.0;
        arpStepCounter = 0;
        lastNoteOffTime = 0.0;
        if (isNoteCurrentlyOn && lastPlayedNote >= 0)
        {
            midiMessages.addEvent(juce::MidiMessage::noteOff(1, lastPlayedNote), 0);
            isNoteCurrentlyOn = false;
        }
        lastPlayedNote = -1;
        return;
    }

    // Arpeggiator is enabled - process incoming MIDI to build held notes list
    juce::MidiBuffer processedMidi;

    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();

        if (message.isNoteOn())
        {
            int note = message.getNoteNumber();
            // Add note to held notes if not already there
            if (std::find(heldNotes.begin(), heldNotes.end(), note) == heldNotes.end())
            {
                bool wasEmpty = heldNotes.empty();
                heldNotes.push_back(note);
                std::sort(heldNotes.begin(), heldNotes.end()); // Keep sorted for Up mode

                // If this is the first note pressed, trigger immediately by setting arpStepTime high
                if (wasEmpty)
                {
                    arpStepTime = 999999.0; // Large value to trigger first note immediately
                    arpStepCounter = 0; // Reset swing counter for new arpeggio sequence
                }
            }
        }
        else if (message.isNoteOff())
        {
            int note = message.getNoteNumber();
            // Remove note from held notes
            heldNotes.erase(std::remove(heldNotes.begin(), heldNotes.end(), note), heldNotes.end());

            // If no notes held, send note-off for last played note
            if (heldNotes.empty())
            {
                if (isNoteCurrentlyOn && lastPlayedNote >= 0)
                {
                    processedMidi.addEvent(juce::MidiMessage::noteOff(1, lastPlayedNote), metadata.samplePosition);
                    isNoteCurrentlyOn = false;
                }
                lastPlayedNote = -1;
                currentArpNote = 0;
                arpStepTime = 0.0;
                arpStepCounter = 0;
            }
        }
        // Don't pass through original note messages when arp is on
    }

    // If progression is enabled and no manual keys are pressed, generate chord notes automatically
    bool progEnabled = parameters.getRawParameterValue(PROG_ENABLED_ID)->load() > 0.5f;
    bool hasManualNotes = false;

    // Check if there are any manually held notes (from MIDI input)
    if (!heldNotes.empty() && !progEnabled)
        hasManualNotes = true;

    if (progEnabled && isPlaybackActive)
    {
        // Check if progression step changed
        if (currentProgressionStep != lastProgressionStepForArp || heldNotes.empty())
        {
            lastProgressionStepForArp = currentProgressionStep;

            // Get current progression value (1-8 representing scale degrees)
            int progressionOffset = getCurrentProgressionOffset();

            // Convert to MIDI note (using sequencer scale and root)
            int rootNote = static_cast<int>(parameters.getRawParameterValue(SEQ_ROOT_ID)->load());
            int scaleType = static_cast<int>(parameters.getRawParameterValue(SEQ_SCALE_ID)->load());

            // Generate a triad (root, 3rd, 5th) based on the progression value
            int baseNote = scaleDegreesToMidiNote(progressionOffset, rootNote, scaleType);
            int third = scaleDegreesToMidiNote((progressionOffset + 2) % 8, rootNote, scaleType);
            int fifth = scaleDegreesToMidiNote((progressionOffset + 4) % 8, rootNote, scaleType);

            // Clear and regenerate chord notes for arpeggiator
            heldNotes.clear();
            heldNotes.push_back(baseNote);
            heldNotes.push_back(third);
            heldNotes.push_back(fifth);
            std::sort(heldNotes.begin(), heldNotes.end());

            // Reset arp to start from beginning with new chord
            currentArpNote = 0;
            arpStepTime = 999999.0; // Trigger immediately
            arpStepCounter = 0;
        }
    }
    else if (!progEnabled)
    {
        // Reset tracking when progression is disabled
        lastProgressionStepForArp = -1;
    }

    // Generate arpeggiated notes only if playback is active
    if (!heldNotes.empty() && isPlaybackActive)
    {
        double baseStepLength = getArpStepLengthInSamples();
        float gateLength = parameters.getRawParameterValue(ARP_GATE_ID)->load();
        int octaveShift = static_cast<int>(parameters.getRawParameterValue(ARP_OCTAVE_SHIFT_ID)->load());
        float swing = parameters.getRawParameterValue(ARP_SWING_ID)->load();

        // Apply swing to step length (alternating between longer and shorter steps)
        // Even steps (0, 2, 4...) get longer, odd steps (1, 3, 5...) get shorter
        double swingAmount = (arpStepCounter % 2 == 0) ? (1.0 + swing * 0.5) : (1.0 - swing * 0.5);
        double stepLength = baseStepLength * swingAmount;
        double noteOffTime = stepLength * gateLength;

        // If arpStepTime was set to trigger immediately, normalize it to stepLength
        if (arpStepTime > stepLength * 2)
        {
            arpStepTime = stepLength;
        }
        else
        {
            arpStepTime += numSamples;
        }

        // Check if we need to turn off the current note
        if (isNoteCurrentlyOn && lastNoteOffTime <= 0 && lastPlayedNote >= 0)
        {
            processedMidi.addEvent(juce::MidiMessage::noteOff(1, lastPlayedNote), 0);
            isNoteCurrentlyOn = false;
        }

        // Check if it's time for the next arp step
        if (arpStepTime >= stepLength)
        {
            // Send note-off for previous note if still on
            if (isNoteCurrentlyOn && lastPlayedNote >= 0)
            {
                processedMidi.addEvent(juce::MidiMessage::noteOff(1, lastPlayedNote), 0);
                isNoteCurrentlyOn = false;
            }

            // Get next note to play
            int noteToPlay = getNextArpNote();

            if (noteToPlay >= 0)
            {
                // Apply octave shift (1 octave = 12 semitones)
                int shiftedNote = noteToPlay + (octaveShift * 12);

                // Constrain to valid MIDI range (0-127)
                shiftedNote = juce::jlimit(0, 127, shiftedNote);

                processedMidi.addEvent(juce::MidiMessage::noteOn(1, shiftedNote, (juce::uint8)100), 0);
                lastPlayedNote = shiftedNote;
                isNoteCurrentlyOn = true;
                lastNoteOffTime = noteOffTime;

                // Increment step counter for swing timing
                arpStepCounter++;
            }

            arpStepTime -= stepLength;
        }

        lastNoteOffTime -= numSamples;
    }

    // Replace MIDI buffer with processed arpeggiator output
    midiMessages.swapWith(processedMidi);
}

double SnorkelSynthAudioProcessor::getArpStepLengthInSamples() const
{
    int rateIndex = static_cast<int>(parameters.getRawParameterValue(ARP_RATE_ID)->load());

    // Rate divisions: 1/32, 1/32., 1/16, 1/16., 1/16T, 1/8, 1/8., 1/8T, 1/4, 1/4., 1/4T, 1/2, 1/2., 1/1
    const double divisions[] = {
        8.0,        // 1/32
        5.333,      // 1/32. (dotted)
        4.0,        // 1/16
        2.667,      // 1/16. (dotted)
        6.0,        // 1/16T (triplet)
        2.0,        // 1/8
        1.333,      // 1/8. (dotted)
        3.0,        // 1/8T (triplet)
        1.0,        // 1/4
        0.667,      // 1/4. (dotted)
        1.5,        // 1/4T (triplet)
        0.5,        // 1/2
        0.333,      // 1/2. (dotted)
        0.25        // 1/1 (whole note)
    };

    rateIndex = juce::jlimit(0, 13, rateIndex);

    double beatsPerSecond = currentBPM / 60.0;
    double notesPerBeat = divisions[rateIndex];
    double stepFrequency = beatsPerSecond * notesPerBeat;

    return currentSampleRate / stepFrequency;
}

int SnorkelSynthAudioProcessor::getNextArpNote()
{
    if (heldNotes.empty())
        return -1;

    int mode = static_cast<int>(parameters.getRawParameterValue(ARP_MODE_ID)->load());
    int octaves = static_cast<int>(parameters.getRawParameterValue(ARP_OCTAVES_ID)->load());

    // Calculate total number of notes across octaves
    int totalNotes = static_cast<int>(heldNotes.size()) * octaves;

    if (totalNotes == 0)
        return -1;

    int noteIndex = 0;

    switch (mode)
    {
        case 0: // Up
            noteIndex = currentArpNote % totalNotes;
            break;

        case 1: // Down
            noteIndex = (totalNotes - 1) - (currentArpNote % totalNotes);
            break;

        case 2: // Up-Down
        {
            int cycle = totalNotes * 2 - 2;
            if (cycle <= 0) cycle = 1;
            int pos = currentArpNote % cycle;
            noteIndex = (pos < totalNotes) ? pos : (cycle - pos);
            break;
        }

        case 3: // Random
            noteIndex = juce::Random::getSystemRandom().nextInt(totalNotes);
            break;

        case 4: // As Played
            noteIndex = currentArpNote % totalNotes;
            break;
    }

    // Convert note index to actual MIDI note with octave
    int baseNoteIndex = noteIndex % static_cast<int>(heldNotes.size());
    int octaveOffset = noteIndex / static_cast<int>(heldNotes.size());
    int midiNote = heldNotes[baseNoteIndex] + (octaveOffset * 12);

    currentArpNote++;

    return juce::jlimit(0, 127, midiNote);
}

//==============================================================================
// Sequencer Implementation

void SnorkelSynthAudioProcessor::processSequencer(juce::MidiBuffer& midiMessages, int numSamples)
{
    bool seqEnabled = parameters.getRawParameterValue(SEQ_ENABLED_ID)->load() > 0.5f;

    // Check if playback is active and sequencer is enabled
    if (!seqEnabled || !isPlaybackActive)
    {
        // Reset sequencer state when disabled
        currentSeqStep = 0;
        seqStepTime = 0.0;
        if (isSeqNoteCurrentlyOn && !lastSeqPlayedNotes.empty())
        {
            for (int note : lastSeqPlayedNotes)
                midiMessages.addEvent(juce::MidiMessage::noteOff(1, note, (juce::uint8)64), 0);
            lastSeqPlayedNotes.clear();
            isSeqNoteCurrentlyOn = false;
        }
        return;
    }

    // Get step length in samples
    double stepLength = getSeqStepLengthInSamples();
    float gateLength = 0.8f; // 80% gate like arp

    juce::MidiBuffer processedMidi;

    // Handle note-off for previous notes
    if (isSeqNoteCurrentlyOn && lastSeqNoteOffTime >= 0.0)
    {
        lastSeqNoteOffTime -= numSamples;
        if (lastSeqNoteOffTime <= 0.0 && !lastSeqPlayedNotes.empty())
        {
            for (int note : lastSeqPlayedNotes)
                processedMidi.addEvent(juce::MidiMessage::noteOff(1, note, (juce::uint8)64), 0);
            lastSeqPlayedNotes.clear();
            isSeqNoteCurrentlyOn = false;
            lastSeqNoteOffTime = -1.0;
        }
    }

    // Advance step time
    seqStepTime += numSamples;

    // Trigger new step
    while (seqStepTime >= stepLength)
    {
        seqStepTime -= stepLength;

        // Get all notes for current step
        std::vector<int> midiNotes = getSequencerNotes(currentSeqStep);

        if (!midiNotes.empty())
        {
            // Trigger note-on for all notes
            for (int midiNote : midiNotes)
                processedMidi.addEvent(juce::MidiMessage::noteOn(1, midiNote, (juce::uint8)100), 0);

            lastSeqPlayedNotes = midiNotes;
            isSeqNoteCurrentlyOn = true;

            // Schedule note-off
            lastSeqNoteOffTime = stepLength * gateLength;
        }

        // Advance to next step (wrap around based on user-defined step count)
        int numSteps = static_cast<int>(parameters.getRawParameterValue(SEQ_STEPS_ID)->load());
        currentSeqStep = (currentSeqStep + 1) % numSteps;
    }

    // Add processed MIDI to output
    for (const auto metadata : processedMidi)
        midiMessages.addEvent(metadata.getMessage(), metadata.samplePosition);

    if (lastSeqNoteOffTime >= 0.0)
        lastSeqNoteOffTime -= numSamples;
}

double SnorkelSynthAudioProcessor::getSeqStepLengthInSamples() const
{
    int rateIndex = static_cast<int>(parameters.getRawParameterValue(SEQ_RATE_ID)->load());

    // Rate divisions: 1/32, 1/32., 1/16, 1/16., 1/16T, 1/8, 1/8., 1/8T, 1/4, 1/4., 1/4T, 1/2, 1/2., 1/1
    const double divisions[] = {
        8.0,        // 1/32
        5.333,      // 1/32. (dotted)
        4.0,        // 1/16
        2.667,      // 1/16. (dotted)
        6.0,        // 1/16T (triplet)
        2.0,        // 1/8
        1.333,      // 1/8. (dotted)
        3.0,        // 1/8T (triplet)
        1.0,        // 1/4
        0.667,      // 1/4. (dotted)
        1.5,        // 1/4T (triplet)
        0.5,        // 1/2
        0.333,      // 1/2. (dotted)
        0.25        // 1/1 (whole note)
    };

    double beatsPerSecond = currentBPM / 60.0;
    double notesPerBeat = divisions[rateIndex];
    double stepFrequency = beatsPerSecond * notesPerBeat;

    return currentSampleRate / stepFrequency;
}

void SnorkelSynthAudioProcessor::processDrums(int numSamples)
{
    bool drumEnabled = parameters.getRawParameterValue("drumenable")->load() > 0.5f;

    // Get sidechain parameters (sqrt curve on magnitude for better response)
    float sidechainMagRaw = parameters.getRawParameterValue("drumsidechainmag")->load();
    float sidechainMag = std::sqrt(sidechainMagRaw); // 50% dial → ~71% ducking
    float sidechainLen = parameters.getRawParameterValue("drumsidechainlen")->load();

    // Decay the sidechain envelope
    // Length controls decay time: 0 = instant, 1 = full step length
    if (sidechainLen > 0.001f)
    {
        double stepLength = getSeqStepLengthInSamples();
        float decayTime = stepLength * sidechainLen * 2.0f; // Up to 2x step length
        float decayRate = numSamples / decayTime;
        sidechainEnvelope = std::max(0.0f, sidechainEnvelope - decayRate);
    }
    else
    {
        sidechainEnvelope = 0.0f;
    }

    if (!drumEnabled || !isPlaybackActive)
    {
        currentDrumStep = 0;
        drumStepTime = 0.0;
        return;
    }

    // Use same step length as sequencer
    double stepLength = getSeqStepLengthInSamples();

    // Advance step time
    drumStepTime += numSamples;

    // Trigger new step
    while (drumStepTime >= stepLength)
    {
        drumStepTime -= stepLength;

        // Check each lane for triggers on this step
        for (int lane = 0; lane < NUM_DRUM_LANES; ++lane)
        {
            if (drumPattern[lane][currentDrumStep] != 0)
            {
                // Trigger this sample
                drumSamplePlaying[lane] = true;
                drumSamplePositions[lane] = 0;

                // Kick triggers sidechain
                if (lane == 0)
                    sidechainEnvelope = sidechainMag;
            }
        }

        // Advance to next step (16 steps)
        currentDrumStep = (currentDrumStep + 1) % NUM_DRUM_STEPS;
    }
}

void SnorkelSynthAudioProcessor::loadDrumSamples()
{
    juce::File dataDir = getDataDirectory();
    juce::File samplesDir = dataDir.getChildFile("samples");

    // Sample file names for each lane
    const juce::String sampleNames[NUM_DRUM_LANES] = {
        "Kick - 9X9.wav",
        "Snare - Snare Hi.wav",
        "HatClosed - Closed Hat.wav",
        "HatOpen - Open Hat.wav"
    };

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    for (int lane = 0; lane < NUM_DRUM_LANES; ++lane)
    {
        juce::File sampleFile = samplesDir.getChildFile(sampleNames[lane]);
        if (sampleFile.existsAsFile())
        {
            std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(sampleFile));
            if (reader != nullptr)
            {
                drumSamples[lane].setSize((int)reader->numChannels, (int)reader->lengthInSamples);
                reader->read(&drumSamples[lane], 0, (int)reader->lengthInSamples, 0, true, true);
            }
        }
    }
}

std::vector<int> SnorkelSynthAudioProcessor::getSequencerNotes(int step)
{
    std::vector<int> notes;

    if (step < 0 || step >= NUM_SEQ_STEPS)
        return notes;

    uint8_t pattern = sequencerPattern[step];
    if (pattern == 0)
        return notes; // No notes at this step

    int rootNote = static_cast<int>(parameters.getRawParameterValue(SEQ_ROOT_ID)->load());
    int scaleType = static_cast<int>(parameters.getRawParameterValue(SEQ_SCALE_ID)->load());
    int progressionOffset = getCurrentProgressionOffset();

    // Check each bit for active degrees
    for (int degree = 0; degree < 8; ++degree)
    {
        if (pattern & (1 << degree))
        {
            int adjustedDegree = (degree + progressionOffset) % 8;
            int midiNote = scaleDegreesToMidiNote(adjustedDegree, rootNote, scaleType);
            // Apply per-note octave shift
            midiNote += sequencerOctave[step][degree] * 12;
            notes.push_back(midiNote);
        }
    }

    return notes;
}

float SnorkelSynthAudioProcessor::getCurrentSeqCutoffMod() const
{
    if (currentSeqStep < 0 || currentSeqStep >= NUM_SEQ_STEPS)
        return 0.0f;

    const char* cutoffParamIds[] = {
        SEQ_CUTOFF1_ID, SEQ_CUTOFF2_ID, SEQ_CUTOFF3_ID, SEQ_CUTOFF4_ID,
        SEQ_CUTOFF5_ID, SEQ_CUTOFF6_ID, SEQ_CUTOFF7_ID, SEQ_CUTOFF8_ID,
        SEQ_CUTOFF9_ID, SEQ_CUTOFF10_ID, SEQ_CUTOFF11_ID, SEQ_CUTOFF12_ID,
        SEQ_CUTOFF13_ID, SEQ_CUTOFF14_ID, SEQ_CUTOFF15_ID, SEQ_CUTOFF16_ID
    };

    return parameters.getRawParameterValue(cutoffParamIds[currentSeqStep])->load();
}

int SnorkelSynthAudioProcessor::scaleDegreesToMidiNote(int scaleDegree, int rootNote, int scaleType)
{
    // Base octave (C3 = MIDI 60)
    const int baseOctave = 60;

    // Scale intervals (semitones from root)
    // scaleDegree: 0-7 where 0=1st, 1=2nd, ..., 6=7th, 7=octave
    static const int scaleIntervals[][8] = {
        {0, 2, 4, 5, 7, 9, 11, 12},  // Major
        {0, 2, 3, 5, 7, 8, 10, 12},  // Minor (Natural)
        {0, 2, 3, 5, 7, 9, 10, 12},  // Dorian
        {0, 1, 3, 5, 7, 8, 10, 12},  // Phrygian
        {0, 2, 4, 6, 7, 9, 11, 12},  // Lydian
        {0, 2, 4, 5, 7, 9, 10, 12},  // Mixolydian
        {0, 2, 3, 5, 7, 8, 10, 12},  // Aeolian (same as Natural Minor)
        {0, 1, 3, 5, 6, 8, 10, 12},  // Locrian
        {0, 2, 3, 5, 7, 8, 11, 12},  // Harmonic Minor
        {0, 2, 3, 5, 7, 9, 11, 12},  // Melodic Minor
        {0, 2, 4, 7, 9, 12, 12, 12}, // Pentatonic Major
        {0, 3, 5, 7, 10, 12, 12, 12},// Pentatonic Minor
        {0, 3, 5, 6, 7, 10, 12, 12}  // Blues
    };

    scaleType = juce::jlimit(0, 12, scaleType);
    scaleDegree = juce::jlimit(0, 7, scaleDegree);

    int midiNote = baseOctave + rootNote + scaleIntervals[scaleType][scaleDegree];

    return juce::jlimit(0, 127, midiNote);
}

//==============================================================================
// Delay Mix LFO Implementation

void SnorkelSynthAudioProcessor::updateDelayMixLFO()
{
    // Read current LFO settings from parameters
    delayMixLFO.rate = static_cast<int>(parameters.getRawParameterValue(DELAYMIX_LFO_RATE_ID)->load());
    delayMixLFO.waveform = static_cast<int>(parameters.getRawParameterValue(DELAYMIX_LFO_WAVE_ID)->load());
    delayMixLFO.depth = parameters.getRawParameterValue(DELAYMIX_LFO_DEPTH_ID)->load();

    // Calculate LFO frequency based on rate (tempo-synced)
    // Rates: 1/16, 1/8, 1/4, 1/3, 1/2, 3/4, 1/1, 3/2, 2/1, 3/1, 4/1, 6/1, 8/1, 12/1, 16/1
    const double rateMultipliers[] = {16.0, 8.0, 4.0, 3.0, 2.0, 1.333, 1.0, 0.667, 0.5, 0.333, 0.25, 0.167, 0.125, 0.083, 0.0625};
    int rateIndex = juce::jlimit(0, 14, delayMixLFO.rate);
    double beatsPerSecond = currentBPM / 60.0;
    delayMixLFO.frequency = beatsPerSecond * rateMultipliers[rateIndex];
}

double SnorkelSynthAudioProcessor::getDelayMixLFOValue()
{
    // Generate LFO value based on waveform type
    double value = 0.0;

    switch (delayMixLFO.waveform)
    {
        case 0: // Sine
            value = std::sin(delayMixLFO.phase);
            break;
        case 1: // Triangle
            value = 2.0 * std::abs(2.0 * (delayMixLFO.phase / juce::MathConstants<double>::twoPi - 0.5)) - 1.0;
            break;
        case 2: // Saw Up
            value = 2.0 * (delayMixLFO.phase / juce::MathConstants<double>::twoPi) - 1.0;
            break;
        case 3: // Saw Down
            value = 1.0 - 2.0 * (delayMixLFO.phase / juce::MathConstants<double>::twoPi);
            break;
        case 4: // Square
            value = (delayMixLFO.phase < juce::MathConstants<double>::pi) ? 1.0 : -1.0;
            break;
        case 5: // Random (sample & hold)
            if (delayMixLFO.phase < juce::MathConstants<double>::pi && delayMixLFO.phase + (delayMixLFO.frequency * juce::MathConstants<double>::twoPi / currentSampleRate) >= juce::MathConstants<double>::pi)
            {
                delayMixLFO.lastRandomValue = (static_cast<float>(rand()) / RAND_MAX) * 2.0f - 1.0f;
            }
            value = delayMixLFO.lastRandomValue;
            break;
    }

    return value; // Returns -1 to +1
}

void SnorkelSynthAudioProcessor::advanceDelayMixLFO()
{
    delayMixLFO.phase += delayMixLFO.frequency * juce::MathConstants<double>::twoPi / currentSampleRate;
    if (delayMixLFO.phase >= juce::MathConstants<double>::twoPi)
        delayMixLFO.phase -= juce::MathConstants<double>::twoPi;
}

//==============================================================================
// Progression Implementation

void SnorkelSynthAudioProcessor::updateProgressionStep(int numSamples)
{
    bool progEnabled = parameters.getRawParameterValue(PROG_ENABLED_ID)->load() > 0.5f;

    if (!progEnabled || !isPlaybackActive)
    {
        progressionBarTime = 0.0;
        currentProgressionStep = 0;
        progressionWasEnabled = false;
        progressionSyncedToBar = false;
        return;
    }

    // Calculate samples per bar (4 beats)
    double samplesPerBar = (currentSampleRate * 60.0 / currentBPM) * 4.0;

    // If progression was just enabled, wait for next bar boundary
    if (!progressionWasEnabled)
    {
        progressionWasEnabled = true;
        progressionSyncedToBar = false;
        progressionBarTime = 0.0;
        currentProgressionStep = 0;
    }

    // Wait for bar boundary before starting
    if (!progressionSyncedToBar)
    {
        // Calculate position within current bar
        double positionInBar = fmod(totalPlaybackTime, samplesPerBar);

        // Check if we're close to a bar boundary (within this buffer)
        if (positionInBar < numSamples || (samplesPerBar - positionInBar) < numSamples)
        {
            progressionSyncedToBar = true;
            progressionBarTime = 0.0;
            currentProgressionStep = 0;
        }
        return; // Don't advance progression until synced
    }

    // Get progression parameters
    int numSteps = static_cast<int>(parameters.getRawParameterValue(PROG_STEPS_ID)->load());
    int lengthIndex = static_cast<int>(parameters.getRawParameterValue(PROG_LENGTH_ID)->load());

    // Convert length index to bar multiplier (0=0.5, 1=1, 2=2, 3=3, 4=4)
    double barMultiplier[] = {0.5, 1.0, 2.0, 3.0, 4.0};
    double stepLengthBars = barMultiplier[lengthIndex];

    // Calculate samples per step
    double samplesPerStep = samplesPerBar * stepLengthBars;

    progressionBarTime += numSamples;

    // Check if we need to advance to next progression step
    if (progressionBarTime >= samplesPerStep)
    {
        progressionBarTime -= samplesPerStep;
        currentProgressionStep = (currentProgressionStep + 1) % numSteps; // Cycle through active steps
    }
}

int SnorkelSynthAudioProcessor::getCurrentProgressionOffset() const
{
    bool progEnabled = parameters.getRawParameterValue(PROG_ENABLED_ID)->load() > 0.5f;

    if (!progEnabled)
        return 0;

    // Get the progression value for the current step (1-16)
    const char* progStepIds[] = {
        PROG_STEP1_ID, PROG_STEP2_ID, PROG_STEP3_ID, PROG_STEP4_ID,
        PROG_STEP5_ID, PROG_STEP6_ID, PROG_STEP7_ID, PROG_STEP8_ID,
        PROG_STEP9_ID, PROG_STEP10_ID, PROG_STEP11_ID, PROG_STEP12_ID,
        PROG_STEP13_ID, PROG_STEP14_ID, PROG_STEP15_ID, PROG_STEP16_ID
    };

    int stepValue = static_cast<int>(parameters.getRawParameterValue(progStepIds[currentProgressionStep])->load());

    // Convert to offset (step 1 = 0 offset, step 2 = 1 offset, etc.)
    return stepValue - 1;
}

//==============================================================================
// Internal playback control (for standalone mode)
void SnorkelSynthAudioProcessor::startPlayback()
{
    isPlaybackActive = true;
    totalPlaybackTime = 0.0;

    // Reset arpeggiator state to start from step 1
    currentArpNote = 0;
    arpStepTime = 0.0;
    arpStepCounter = 0;
    lastNoteOffTime = 0.0;
    lastProgressionStepForArp = -1; // Force chord generation on first step

    // Reset sequencer state to start from step 1
    currentSeqStep = 0;
    seqStepTime = 0.0;
    lastSeqNoteOffTime = 0.0;

    // Reset progression state to start from step 1
    progressionBarTime = 0.0;
    currentProgressionStep = 0;
    progressionWasEnabled = true; // Prevent the reset logic in updateProgressionStep
    progressionSyncedToBar = true; // Don't wait for bar sync, start immediately
}

void SnorkelSynthAudioProcessor::stopPlayback()
{
    isPlaybackActive = false;
    totalPlaybackTime = 0.0; // Reset playback timer

    // Stop any currently playing arpeggiator notes
    if (isNoteCurrentlyOn && lastPlayedNote >= 0)
    {
        synth.noteOff(1, lastPlayedNote, 0.0f, true);
    }
    lastPlayedNote = -1;
    isNoteCurrentlyOn = false;
    heldNotes.clear();
    currentArpNote = 0;
    arpStepTime = 0.0;
    arpStepCounter = 0;
    lastNoteOffTime = 0.0;
    lastProgressionStepForArp = -1;

    // Stop any currently playing sequencer notes
    if (isSeqNoteCurrentlyOn && lastSeqPlayedNote >= 0)
    {
        synth.noteOff(1, lastSeqPlayedNote, 0.0f, true);
    }
    lastSeqPlayedNote = -1;
    isSeqNoteCurrentlyOn = false;
    currentSeqStep = 0;
    seqStepTime = 0.0;
    lastSeqNoteOffTime = 0.0;

    // Reset progression state
    progressionBarTime = 0.0;
    currentProgressionStep = 0;
    progressionWasEnabled = false;
    progressionSyncedToBar = false;
}

void SnorkelSynthAudioProcessor::showEditorMessage(const juce::String& message)
{
    if (currentEditor != nullptr)
    {
        // Call showMessage on the editor - defined in PluginEditor.h
        currentEditor->showMessage(message);
    }
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SnorkelSynthAudioProcessor();
}
