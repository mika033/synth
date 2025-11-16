#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// CONFIGURATION - Edit these constants to customize the plugin
//==============================================================================

// Plugin Identity
static constexpr const char* kPluginName = "Acid Synth";
static constexpr const char* kPluginVersion = "1.0.0";
static constexpr const char* kManufacturerName = "AcidLab";

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
    static constexpr int   kDelayTime = 3;  // 1/8 note (index 3)
    static constexpr float kDelayFeedback = 0.3f;
    static constexpr float kDelayMix = 0.0f; // Off by default

    // Dedicated LFO defaults (each parameter has its own LFO)
    static constexpr int   kLFORate = 6;     // Default: 1/1 (whole note)
    static constexpr int   kLFOWaveform = 0; // Default: Sine wave
    static constexpr float kLFODepth = 0.0f; // Default: Off
}

//==============================================================================
// Factory Presets
static const Preset kPresets[] = {
    // Classic 303 Bass (Fat & Punchy) - No LFOs for traditional sound
    { "Classic 303 Bass",
      750.0f, 0.8f, 0.6f, 0.2f, 0.7f, 0.0f, 0.5f, 0.3f, 0.7f,
      6, 0.3f, 0.0f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Squelchy Lead (Maximum Expression) - Cutoff LFO for movement
    { "Squelchy Lead",
      400.0f, 0.9f, 0.9f, 0.75f, 0.85f, 1.0f, 0.25f, 0.6f, 0.6f,
      4, 0.4f, 0.2f,
      {1, 0, 0.7f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Deep Rumble (Sub-Heavy) - Volume LFO for subtle pulse
    { "Deep Rumble",
      300.0f, 0.6f, 0.4f, 0.6f, 0.4f, 0.0f, 0.8f, 0.2f, 0.5f,
      9, 0.2f, 0.0f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {2, 0, 0.3f}, {6, 0, 0.0f}
    },

    // Aggressive Distorted Lead - Resonance LFO for aggression
    { "Aggressive Lead",
      1150.0f, 0.9f, 0.8f, 0.35f, 0.9f, 1.0f, 0.1f, 0.85f, 0.6f,
      7, 0.5f, 0.3f,
      {6, 0, 0.0f}, {1, 0, 0.6f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Pulsing Bass - Cutoff LFO at 1/8 rate
    { "Pulsing Bass",
      600.0f, 0.75f, 0.7f, 0.4f, 0.6f, 0.0f, 0.6f, 0.2f, 0.65f,
      6, 0.3f, 0.1f,
      {1, 0, 0.8f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Dub Delay Bass - No LFOs (relies on delay effect)
    { "Dub Delay Bass",
      500.0f, 0.7f, 0.5f, 0.35f, 0.5f, 0.0f, 0.7f, 0.1f, 0.6f,
      7, 0.6f, 0.4f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Wobble Bass - Resonance LFO at faster rate
    { "Wobble Bass",
      800.0f, 0.85f, 0.8f, 0.3f, 0.7f, 0.0f, 0.4f, 0.3f, 0.7f,
      3, 0.2f, 0.0f,
      {6, 0, 0.0f}, {1, 0, 0.75f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Soft Pad - Volume LFO for subtle modulation
    { "Soft Pad",
      1200.0f, 0.5f, 0.3f, 0.8f, 0.2f, 1.0f, 0.3f, 0.0f, 0.5f,
      10, 0.7f, 0.5f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {2, 0, 0.4f}, {6, 0, 0.0f}
    },

    // ===== TRADITIONAL SYNTH PRESETS =====

    // Smooth Lead - Melodic lead with morphed waveform, moderate filter, no LFOs
    { "Smooth Lead",
      1800.0f, 0.4f, 0.5f, 0.4f, 0.3f, 0.5f, 0.2f, 0.2f, 0.65f,
      6, 0.2f, 0.1f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Warm Bass - Fat traditional bass with sub-oscillator, low resonance
    { "Warm Bass",
      600.0f, 0.3f, 0.4f, 0.35f, 0.4f, 0.2f, 0.7f, 0.15f, 0.7f,
      6, 0.25f, 0.0f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Evolving Pad - Morphing waveform with gentle LFO for evolving texture
    { "Evolving Pad",
      2000.0f, 0.35f, 0.3f, 0.7f, 0.2f, 0.6f, 0.25f, 0.05f, 0.55f,
      10, 0.4f, 0.35f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {5, 0, 0.25f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    },

    // Init (default clean sound) - All LFOs disabled - MOVED TO BOTTOM
    { "Init",
      1000.0f, 0.7f, 0.5f, 0.3f, 0.5f, 0.0f, 0.5f, 0.0f, 0.7f,
      6, 0.3f, 0.0f,
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f},
      {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}, {6, 0, 0.0f}
    }
};

static constexpr int kNumPresets = sizeof(kPresets) / sizeof(Preset);

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
AcidSynthAudioProcessor::AcidSynthAudioProcessor()
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
                        DECAY_ID, "Decay",
                        juce::NormalisableRange<float>(0.01f, 2.0f, 0.01f, 0.5f),
                        Defaults::kDecay),

                    std::make_unique<juce::AudioParameterFloat>(
                        ACCENT_ID, "Accent",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kAccent),

                    std::make_unique<juce::AudioParameterFloat>(
                        WAVEFORM_ID, "Waveform",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kWaveform),

                    std::make_unique<juce::AudioParameterFloat>(
                        SUB_OSC_ID, "Sub Osc",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kSubOsc),

                    std::make_unique<juce::AudioParameterFloat>(
                        DRIVE_ID, "Drive",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kDrive),

                    std::make_unique<juce::AudioParameterFloat>(
                        VOLUME_ID, "Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kVolume),

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
                        ARP_ONOFF_ID, "Arp On/Off", false),
                    std::make_unique<juce::AudioParameterChoice>(
                        ARP_MODE_ID, "Arp Mode",
                        juce::StringArray{"Up", "Down", "Up-Down", "Random", "As Played"}, 0),
                    std::make_unique<juce::AudioParameterChoice>(
                        ARP_RATE_ID, "Arp Rate",
                        juce::StringArray{"1/32", "1/16", "1/16T", "1/8", "1/8T", "1/4", "1/4T"}, 3), // Default to 1/8
                    std::make_unique<juce::AudioParameterInt>(
                        ARP_OCTAVES_ID, "Arp Octaves",
                        1, 4, 1), // 1-4 octaves
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
                        0.0f) // Default 0% swing (straight timing)
                })
{
    // Add voices to the synthesizer
    for (int i = 0; i < kNumVoices; ++i)
        synth.addVoice(new AcidVoice());

    // Add sound
    synth.addSound(new AcidSound());
}

AcidSynthAudioProcessor::~AcidSynthAudioProcessor()
{
}

//==============================================================================
const juce::String AcidSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AcidSynthAudioProcessor::acceptsMidi() const
{
    return true;
}

bool AcidSynthAudioProcessor::producesMidi() const
{
    return false;
}

bool AcidSynthAudioProcessor::isMidiEffect() const
{
    return false;
}

double AcidSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AcidSynthAudioProcessor::getNumPrograms()
{
    return kNumPresets;
}

int AcidSynthAudioProcessor::getCurrentProgram()
{
    return 0; // Could track current preset if needed
}

void AcidSynthAudioProcessor::setCurrentProgram(int index)
{
    if (index >= 0 && index < kNumPresets)
        loadPreset(index);
}

const juce::String AcidSynthAudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < kNumPresets)
        return kPresets[index].name;
    return {};
}

void AcidSynthAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
    // Factory presets cannot be renamed
}

//==============================================================================
void AcidSynthAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
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

void AcidSynthAudioProcessor::releaseResources()
{
}

bool AcidSynthAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void AcidSynthAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
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
        }
    }

    // Use default BPM if no host BPM available
    if (!bpmFromHost)
    {
        currentBPM = DEFAULT_BPM;
    }

    // Handle internal playback (for standalone mode)
    if (isInternalPlaybackActive)
    {
        // Inject C3 note-on message at the start of the buffer
        midiMessages.addEvent(juce::MidiMessage::noteOn(1, INTERNAL_PLAYBACK_NOTE, (juce::uint8)100), 0);
    }

    // Clear output buffer
    buffer.clear();

    // Update voice parameters
    updateVoiceParameters();

    // Process arpeggiator (modifies MIDI messages)
    processArpeggiator(midiMessages, buffer.getNumSamples());

    // Render synthesizer
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

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
}

void AcidSynthAudioProcessor::updateVoiceParameters()
{
    // Main parameters
    float cutoff = parameters.getRawParameterValue(CUTOFF_ID)->load();
    float resonance = parameters.getRawParameterValue(RESONANCE_ID)->load();
    float envMod = parameters.getRawParameterValue(ENV_MOD_ID)->load();
    float decay = parameters.getRawParameterValue(DECAY_ID)->load();
    float accent = parameters.getRawParameterValue(ACCENT_ID)->load();
    float waveform = parameters.getRawParameterValue(WAVEFORM_ID)->load(); // Now float for morphing
    float subOsc = parameters.getRawParameterValue(SUB_OSC_ID)->load();
    float drive = parameters.getRawParameterValue(DRIVE_ID)->load();
    float volume = parameters.getRawParameterValue(VOLUME_ID)->load();
    float filterFeedback = parameters.getRawParameterValue(FILTER_FEEDBACK_ID)->load();
    int saturationType = static_cast<int>(parameters.getRawParameterValue(SATURATION_TYPE_ID)->load());

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

    // Update all voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<AcidVoice*>(synth.getVoice(i)))
        {
            // Set main parameters
            voice->setCutoff(cutoff);
            voice->setResonance(resonance);
            voice->setEnvMod(envMod);
            voice->setDecay(decay);
            voice->setAccent(accent);
            voice->setWaveform(waveform);
            voice->setSubOscMix(subOsc);
            voice->setDrive(drive);
            voice->setVolume(volume);
            voice->setBPM(currentBPM);
            voice->setFilterFeedback(filterFeedback);
            voice->setSaturationType(saturationType);

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

void AcidSynthAudioProcessor::loadPreset(int presetIndex)
{
    if (presetIndex < 0 || presetIndex >= kNumPresets)
        return;

    const Preset& preset = kPresets[presetIndex];

    // Set all parameters to preset values
    parameters.getParameter(CUTOFF_ID)->setValueNotifyingHost(
        parameters.getParameterRange(CUTOFF_ID).convertTo0to1(preset.cutoff));

    parameters.getParameter(RESONANCE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(RESONANCE_ID).convertTo0to1(preset.resonance));

    parameters.getParameter(ENV_MOD_ID)->setValueNotifyingHost(
        parameters.getParameterRange(ENV_MOD_ID).convertTo0to1(preset.envMod));

    parameters.getParameter(DECAY_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DECAY_ID).convertTo0to1(preset.decay));

    parameters.getParameter(ACCENT_ID)->setValueNotifyingHost(
        parameters.getParameterRange(ACCENT_ID).convertTo0to1(preset.accent));

    parameters.getParameter(WAVEFORM_ID)->setValueNotifyingHost(preset.waveform);

    parameters.getParameter(SUB_OSC_ID)->setValueNotifyingHost(
        parameters.getParameterRange(SUB_OSC_ID).convertTo0to1(preset.subOsc));

    parameters.getParameter(DRIVE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DRIVE_ID).convertTo0to1(preset.drive));

    parameters.getParameter(VOLUME_ID)->setValueNotifyingHost(
        parameters.getParameterRange(VOLUME_ID).convertTo0to1(preset.volume));

    // NOTE: LFO settings are NOT loaded from presets - they are left untouched
    // This allows users to set up their LFO configurations independently of presets

    parameters.getParameter(DELAY_TIME_ID)->setValueNotifyingHost(
        static_cast<float>(preset.delayTime));

    parameters.getParameter(DELAY_FEEDBACK_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DELAY_FEEDBACK_ID).convertTo0to1(preset.delayFeedback));

    parameters.getParameter(DELAY_MIX_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DELAY_MIX_ID).convertTo0to1(preset.delayMix));
}

//==============================================================================
bool AcidSynthAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AcidSynthAudioProcessor::createEditor()
{
    return new AcidSynthAudioProcessorEditor(*this);
}

//==============================================================================
void AcidSynthAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void AcidSynthAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// Arpeggiator Implementation

void AcidSynthAudioProcessor::processArpeggiator(juce::MidiBuffer& midiMessages, int numSamples)
{
    bool arpEnabled = parameters.getRawParameterValue(ARP_ONOFF_ID)->load() > 0.5f;

    if (!arpEnabled)
    {
        // Arpeggiator is off, clear state and pass through MIDI
        heldNotes.clear();
        currentArpNote = 0;
        arpStepTime = 0.0;
        arpStepCounter = 0;
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

    // Generate arpeggiated notes
    if (!heldNotes.empty())
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

double AcidSynthAudioProcessor::getArpStepLengthInSamples() const
{
    int rateIndex = static_cast<int>(parameters.getRawParameterValue(ARP_RATE_ID)->load());

    // Rate divisions: 1/32, 1/16, 1/16T, 1/8, 1/8T, 1/4, 1/4T
    const double divisions[] = {
        8.0,        // 1/32
        4.0,        // 1/16
        6.0,        // 1/16T (triplet)
        2.0,        // 1/8
        3.0,        // 1/8T (triplet)
        1.0,        // 1/4
        1.5         // 1/4T (triplet)
    };

    rateIndex = juce::jlimit(0, 6, rateIndex);

    double beatsPerSecond = currentBPM / 60.0;
    double notesPerBeat = divisions[rateIndex];
    double stepFrequency = beatsPerSecond * notesPerBeat;

    return currentSampleRate / stepFrequency;
}

int AcidSynthAudioProcessor::getNextArpNote()
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
// Delay Mix LFO Implementation

void AcidSynthAudioProcessor::updateDelayMixLFO()
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

double AcidSynthAudioProcessor::getDelayMixLFOValue()
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

void AcidSynthAudioProcessor::advanceDelayMixLFO()
{
    delayMixLFO.phase += delayMixLFO.frequency * juce::MathConstants<double>::twoPi / currentSampleRate;
    if (delayMixLFO.phase >= juce::MathConstants<double>::twoPi)
        delayMixLFO.phase -= juce::MathConstants<double>::twoPi;
}

//==============================================================================
// Internal playback control (for standalone mode)
void AcidSynthAudioProcessor::startInternalPlayback()
{
    isInternalPlaybackActive = true;
}

void AcidSynthAudioProcessor::stopInternalPlayback()
{
    isInternalPlaybackActive = false;

    // Send note-off for the internal playback note to stop any held notes
    juce::MidiBuffer stopMessages;
    stopMessages.addEvent(juce::MidiMessage::noteOff(1, INTERNAL_PLAYBACK_NOTE), 0);

    // Clear the held notes in arpeggiator
    heldNotes.clear();
    currentArpNote = 0;
    arpStepTime = 0.0;
    if (isNoteCurrentlyOn && lastPlayedNote >= 0)
    {
        synth.noteOff(1, lastPlayedNote, 0.0f, true);
    }
    lastPlayedNote = -1;
    isNoteCurrentlyOn = false;
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AcidSynthAudioProcessor();
}
