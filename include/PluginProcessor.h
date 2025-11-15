#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include "AcidVoice.h"

//==============================================================================
/**
 * Main audio processor for Acid Synth VST plugin
 */
class AcidSynthAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    AcidSynthAudioProcessor();
    ~AcidSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    // Parameter access
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }

private:
    //==============================================================================
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState parameters;

    // Parameter IDs
    static constexpr const char* CUTOFF_ID = "cutoff";
    static constexpr const char* RESONANCE_ID = "resonance";
    static constexpr const char* ENV_MOD_ID = "envmod";
    static constexpr const char* DECAY_ID = "decay";
    static constexpr const char* ACCENT_ID = "accent";
    static constexpr const char* WAVEFORM_ID = "waveform";
    static constexpr const char* SUB_OSC_ID = "subosc";
    static constexpr const char* DRIVE_ID = "drive";
    static constexpr const char* VOLUME_ID = "volume";
    static constexpr const char* LFO_RATE_ID = "lforate";
    static constexpr const char* LFO_DEST_ID = "lfodest";
    static constexpr const char* LFO_DEPTH_ID = "lfodepth";
    static constexpr const char* DELAY_TIME_ID = "delaytime";
    static constexpr const char* DELAY_FEEDBACK_ID = "delayfeedback";
    static constexpr const char* DELAY_MIX_ID = "delaymix";

    // Parameter update
    void updateVoiceParameters();

    // Preset management
    void loadPreset(int presetIndex);

    // Delay effect
    juce::dsp::DelayLine<float> delayLine { 192000 }; // Max 4 seconds at 48kHz
    std::vector<float> delayBuffer;
    double currentSampleRate = 44100.0;
    double currentBPM = 120.0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcidSynthAudioProcessor)
};

// Preset structure
struct Preset
{
    juce::String name;
    float cutoff;
    float resonance;
    float envMod;
    float decay;
    float accent;
    int waveform;  // 0 = saw, 1 = square
    float subOsc;
    float drive;
    float volume;
    int lfoRate;     // 0=1/16, 1=1/8, 2=1/4, 3=1/2, 4=1/1
    int lfoDest;     // 0=Off, 1=Cutoff, 2=Resonance, 3=Volume
    float lfoDepth;
    int delayTime;   // 0=1/16, 1=1/8, 2=1/4, 3=1/2, 4=1/1
    float delayFeedback;
    float delayMix;
};
