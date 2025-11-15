#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// Factory Presets
static const Preset kPresets[] = {
    // Classic 303 Bass (Fat & Punchy)
    { "Classic 303 Bass", 750.0f, 0.8f, 0.6f, 0.2f, 0.7f, 0, 0.5f, 0.3f, 0.7f },

    // Squelchy Lead (Maximum Expression)
    { "Squelchy Lead", 400.0f, 0.9f, 0.9f, 0.75f, 0.85f, 1, 0.25f, 0.6f, 0.6f },

    // Deep Rumble (Sub-Heavy)
    { "Deep Rumble", 300.0f, 0.6f, 0.4f, 0.6f, 0.4f, 0, 0.8f, 0.2f, 0.5f },

    // Aggressive Distorted Lead
    { "Aggressive Lead", 1150.0f, 0.9f, 0.8f, 0.35f, 0.9f, 1, 0.1f, 0.85f, 0.6f },

    // Init (default clean sound)
    { "Init", 1000.0f, 0.7f, 0.5f, 0.3f, 0.5f, 0, 0.5f, 0.0f, 0.7f }
};

static constexpr int kNumPresets = sizeof(kPresets) / sizeof(Preset);

//==============================================================================
AcidSynthAudioProcessor::AcidSynthAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS",
                {
                    std::make_unique<juce::AudioParameterFloat>(
                        CUTOFF_ID, "Cutoff",
                        juce::NormalisableRange<float>(20.0f, 5000.0f, 1.0f, 0.3f),
                        1000.0f),

                    std::make_unique<juce::AudioParameterFloat>(
                        RESONANCE_ID, "Resonance",
                        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f),
                        0.7f),

                    std::make_unique<juce::AudioParameterFloat>(
                        ENV_MOD_ID, "Env Mod",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f),

                    std::make_unique<juce::AudioParameterFloat>(
                        DECAY_ID, "Decay",
                        juce::NormalisableRange<float>(0.01f, 2.0f, 0.01f, 0.5f),
                        0.3f),

                    std::make_unique<juce::AudioParameterFloat>(
                        ACCENT_ID, "Accent",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f),

                    std::make_unique<juce::AudioParameterChoice>(
                        WAVEFORM_ID, "Waveform",
                        juce::StringArray{"Saw", "Square"},
                        0),

                    std::make_unique<juce::AudioParameterFloat>(
                        SUB_OSC_ID, "Sub Osc",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f),

                    std::make_unique<juce::AudioParameterFloat>(
                        DRIVE_ID, "Drive",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.0f),

                    std::make_unique<juce::AudioParameterFloat>(
                        VOLUME_ID, "Volume",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.7f)
                })
{
    // Add 8 voices to the synthesizer
    for (int i = 0; i < 8; ++i)
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

    // Clear output buffer
    buffer.clear();

    // Update voice parameters
    updateVoiceParameters();

    // Render synthesizer
    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
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
