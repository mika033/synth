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
    static constexpr int   kDelayTime = 6;  // 1/4 note
    static constexpr float kDelayFeedback = 0.3f;
    static constexpr float kDelayMix = 0.0f; // Off by default

    // Dedicated LFO defaults (each parameter has its own LFO)
    static constexpr int   kLFORate = 4;     // Default: 1/4 note (index 4 in expanded list)
    static constexpr int   kLFOWaveform = 0; // Default: Sine wave
    static constexpr float kLFODepth = 0.0f; // Default: Off
}

//==============================================================================
// Factory Presets
static const Preset kPresets[] = {
    // Name, Cutoff, Resonance, EnvMod, Decay, Accent, Waveform, SubOsc, Drive, Volume, LFORate, LFODest, LFODepth, DelayTime, DelayFeedback, DelayMix

    // Classic 303 Bass (Fat & Punchy)
    { "Classic 303 Bass", 750.0f, 0.8f, 0.6f, 0.2f, 0.7f, 0, 0.5f, 0.3f, 0.7f, 2, 0, 0.5f, 6, 0.3f, 0.0f },

    // Squelchy Lead (Maximum Expression)
    { "Squelchy Lead", 400.0f, 0.9f, 0.9f, 0.75f, 0.85f, 1, 0.25f, 0.6f, 0.6f, 2, 1, 0.7f, 4, 0.4f, 0.2f },

    // Deep Rumble (Sub-Heavy)
    { "Deep Rumble", 300.0f, 0.6f, 0.4f, 0.6f, 0.4f, 0, 0.8f, 0.2f, 0.5f, 3, 0, 0.3f, 9, 0.2f, 0.0f },

    // Aggressive Distorted Lead
    { "Aggressive Lead", 1150.0f, 0.9f, 0.8f, 0.35f, 0.9f, 1, 0.1f, 0.85f, 0.6f, 1, 2, 0.6f, 7, 0.5f, 0.3f },

    // Pulsing Bass (LFO Cutoff)
    { "Pulsing Bass", 600.0f, 0.75f, 0.7f, 0.4f, 0.6f, 0, 0.6f, 0.2f, 0.65f, 2, 1, 0.8f, 6, 0.3f, 0.1f },

    // Dub Delay Bass
    { "Dub Delay Bass", 500.0f, 0.7f, 0.5f, 0.35f, 0.5f, 0, 0.7f, 0.1f, 0.6f, 2, 0, 0.3f, 7, 0.6f, 0.4f },

    // Wobble Bass (LFO Resonance)
    { "Wobble Bass", 800.0f, 0.85f, 0.8f, 0.3f, 0.7f, 0, 0.4f, 0.3f, 0.7f, 1, 2, 0.75f, 3, 0.2f, 0.0f },

    // Soft Pad (Square + Delay)
    { "Soft Pad", 1200.0f, 0.5f, 0.3f, 0.8f, 0.2f, 1, 0.3f, 0.0f, 0.5f, 3, 3, 0.4f, 10, 0.7f, 0.5f },

    // Init (default clean sound)
    { "Init", 1000.0f, 0.7f, 0.5f, 0.3f, 0.5f, 0, 0.5f, 0.0f, 0.7f, 2, 0, 0.5f, 6, 0.3f, 0.0f }
};

static constexpr int kNumPresets = sizeof(kPresets) / sizeof(Preset);

//==============================================================================
// LFO Rate and Waveform Options
static const juce::StringArray getLFORateOptions()
{
    return juce::StringArray{"1/16", "1/8", "1/4", "1/3", "1/2", "3/4", "1/1", "2/1", "3/1", "4/1", "6/1", "8/1", "12/1", "16/1"};
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
                        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
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
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        Defaults::kLFODepth)
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

    // Get BPM from host
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
            {
                currentBPM = *positionInfo->getBpm();
            }
        }
    }

    // Clear output buffer
    buffer.clear();

    // Update voice parameters
    updateVoiceParameters();

    // Render synthesizer
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply delay effect
    float delayMix = parameters.getRawParameterValue(DELAY_MIX_ID)->load();

    if (delayMix > 0.001f)
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
                // Read delayed sample
                float delayedSample = delayLine.popSample(channel);

                // Mix input with feedback
                float inputSample = channelData[sample];
                float feedbackSample = inputSample + delayedSample * delayFeedback;

                // Push to delay line
                delayLine.pushSample(channel, feedbackSample);

                // Mix dry and wet
                channelData[sample] = inputSample * (1.0f - delayMix) + delayedSample * delayMix;
            }
        }
    }
}

void AcidSynthAudioProcessor::updateVoiceParameters()
{
    float cutoff = parameters.getRawParameterValue(CUTOFF_ID)->load();
    float resonance = parameters.getRawParameterValue(RESONANCE_ID)->load();
    float envMod = parameters.getRawParameterValue(ENV_MOD_ID)->load();
    float decay = parameters.getRawParameterValue(DECAY_ID)->load();
    float accent = parameters.getRawParameterValue(ACCENT_ID)->load();
    int waveform = static_cast<int>(parameters.getRawParameterValue(WAVEFORM_ID)->load());
    float subOsc = parameters.getRawParameterValue(SUB_OSC_ID)->load();
    float drive = parameters.getRawParameterValue(DRIVE_ID)->load();
    float volume = parameters.getRawParameterValue(VOLUME_ID)->load();
    int lfoRate = static_cast<int>(parameters.getRawParameterValue(LFO_RATE_ID)->load());
    int lfoDest = static_cast<int>(parameters.getRawParameterValue(LFO_DEST_ID)->load());
    float lfoDepth = parameters.getRawParameterValue(LFO_DEPTH_ID)->load();

    // Update all voices
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<AcidVoice*>(synth.getVoice(i)))
        {
            voice->setCutoff(cutoff);
            voice->setResonance(resonance);
            voice->setEnvMod(envMod);
            voice->setDecay(decay);
            voice->setAccent(accent);
            voice->setWaveform(waveform);
            voice->setSubOscMix(subOsc);
            voice->setDrive(drive);
            voice->setVolume(volume);
            voice->setLFORate(lfoRate);
            voice->setLFODestination(lfoDest);
            voice->setLFODepth(lfoDepth);
            voice->setBPM(currentBPM);
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

    parameters.getParameter(WAVEFORM_ID)->setValueNotifyingHost(
        static_cast<float>(preset.waveform));

    parameters.getParameter(SUB_OSC_ID)->setValueNotifyingHost(
        parameters.getParameterRange(SUB_OSC_ID).convertTo0to1(preset.subOsc));

    parameters.getParameter(DRIVE_ID)->setValueNotifyingHost(
        parameters.getParameterRange(DRIVE_ID).convertTo0to1(preset.drive));

    parameters.getParameter(VOLUME_ID)->setValueNotifyingHost(
        parameters.getParameterRange(VOLUME_ID).convertTo0to1(preset.volume));

    parameters.getParameter(LFO_RATE_ID)->setValueNotifyingHost(
        static_cast<float>(preset.lfoRate));

    parameters.getParameter(LFO_DEST_ID)->setValueNotifyingHost(
        static_cast<float>(preset.lfoDest));

    parameters.getParameter(LFO_DEPTH_ID)->setValueNotifyingHost(
        parameters.getParameterRange(LFO_DEPTH_ID).convertTo0to1(preset.lfoDepth));

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
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AcidSynthAudioProcessor();
}
