#include "PluginProcessor.h"
#include "PluginEditor.h"

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
                        0)
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
    return 1;
}

int AcidSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AcidSynthAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String AcidSynthAudioProcessor::getProgramName(int index)
{
    return {};
}

void AcidSynthAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
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
        }
    }
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
