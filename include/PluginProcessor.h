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

    // Main Parameter IDs
    static constexpr const char* CUTOFF_ID = "cutoff";
    static constexpr const char* RESONANCE_ID = "resonance";
    static constexpr const char* ENV_MOD_ID = "envmod";
    static constexpr const char* DECAY_ID = "decay";
    static constexpr const char* ACCENT_ID = "accent";
    static constexpr const char* WAVEFORM_ID = "waveform";
    static constexpr const char* SUB_OSC_ID = "subosc";
    static constexpr const char* DRIVE_ID = "drive";
    static constexpr const char* VOLUME_ID = "volume";
    static constexpr const char* DELAY_TIME_ID = "delaytime";
    static constexpr const char* DELAY_FEEDBACK_ID = "delayfeedback";
    static constexpr const char* DELAY_MIX_ID = "delaymix";

    // Dedicated LFO Parameter IDs (Rate, Waveform, Depth for each parameter)
    static constexpr const char* CUTOFF_LFO_RATE_ID = "cutofflfor";
    static constexpr const char* CUTOFF_LFO_WAVE_ID = "cutofflfow";
    static constexpr const char* CUTOFF_LFO_DEPTH_ID = "cutofflfod";

    static constexpr const char* RESONANCE_LFO_RATE_ID = "resonancelforate";
    static constexpr const char* RESONANCE_LFO_WAVE_ID = "resonancelfowave";
    static constexpr const char* RESONANCE_LFO_DEPTH_ID = "resonancelfodepth";

    static constexpr const char* ENVMOD_LFO_RATE_ID = "envmodlforate";
    static constexpr const char* ENVMOD_LFO_WAVE_ID = "envmodlfowave";
    static constexpr const char* ENVMOD_LFO_DEPTH_ID = "envmodlfodepth";

    static constexpr const char* DECAY_LFO_RATE_ID = "decaylforate";
    static constexpr const char* DECAY_LFO_WAVE_ID = "decaylfowave";
    static constexpr const char* DECAY_LFO_DEPTH_ID = "decaylfodepth";

    static constexpr const char* ACCENT_LFO_RATE_ID = "accentlforate";
    static constexpr const char* ACCENT_LFO_WAVE_ID = "accentlfowave";
    static constexpr const char* ACCENT_LFO_DEPTH_ID = "accentlfodepth";

    static constexpr const char* WAVEFORM_LFO_RATE_ID = "waveformlforate";
    static constexpr const char* WAVEFORM_LFO_WAVE_ID = "waveformlfowave";
    static constexpr const char* WAVEFORM_LFO_DEPTH_ID = "waveformlfodepth";

    static constexpr const char* SUBOSC_LFO_RATE_ID = "subosclforate";
    static constexpr const char* SUBOSC_LFO_WAVE_ID = "subosclfowave";
    static constexpr const char* SUBOSC_LFO_DEPTH_ID = "subosclfodepth";

    static constexpr const char* DRIVE_LFO_RATE_ID = "drivelforate";
    static constexpr const char* DRIVE_LFO_WAVE_ID = "drivelfowave";
    static constexpr const char* DRIVE_LFO_DEPTH_ID = "drivelfodepth";

    static constexpr const char* VOLUME_LFO_RATE_ID = "volumelforate";
    static constexpr const char* VOLUME_LFO_WAVE_ID = "volumelfowave";
    static constexpr const char* VOLUME_LFO_DEPTH_ID = "volumelfodepth";

    static constexpr const char* DELAYMIX_LFO_RATE_ID = "delaymixlforate";
    static constexpr const char* DELAYMIX_LFO_WAVE_ID = "delaymixlfowave";
    static constexpr const char* DELAYMIX_LFO_DEPTH_ID = "delaymixlfodepth";

    // Arpeggiator Parameter IDs
    static constexpr const char* ARP_ONOFF_ID = "arponoff";
    static constexpr const char* ARP_MODE_ID = "arpmode";
    static constexpr const char* ARP_RATE_ID = "arprate";
    static constexpr const char* ARP_OCTAVES_ID = "arpoctaves";
    static constexpr const char* ARP_GATE_ID = "arpgate";

    // Parameter update
    void updateVoiceParameters();

    // Preset management
    void loadPreset(int presetIndex);

    // Delay effect
    juce::dsp::DelayLine<float> delayLine { 192000 }; // Max 4 seconds at 48kHz
    std::vector<float> delayBuffer;
    double currentSampleRate = 44100.0;
    double currentBPM = 120.0;

    // Arpeggiator state
    std::vector<int> heldNotes; // Currently held MIDI notes
    int currentArpNote = 0;      // Index into heldNotes
    double arpStepTime = 0.0;    // Time since last arp step (in samples)
    double lastNoteOffTime = 0.0; // Time for note-off based on gate length
    int lastPlayedNote = -1;     // Last arpeggio note that was triggered
    bool isNoteCurrentlyOn = false; // Track if we're in note-on phase

    // Arpeggiator helper functions
    void processArpeggiator(juce::MidiBuffer& midiMessages, int numSamples);
    double getArpStepLengthInSamples() const;
    int getNextArpNote();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcidSynthAudioProcessor)
};

// LFO settings structure
struct LFOSettings
{
    int rate;      // Index into rate options
    int waveform;  // 0=Sine, 1=Triangle, 2=SawUp, 3=SawDown, 4=Square, 5=Random
    float depth;   // 0.0-1.0
};

// Preset structure
struct Preset
{
    juce::String name;

    // Main parameters
    float cutoff;
    float resonance;
    float envMod;
    float decay;
    float accent;
    float waveform;  // 0.0=saw, 1.0=square, morph in between
    float subOsc;
    float drive;
    float volume;

    // Delay parameters
    int delayTime;
    float delayFeedback;
    float delayMix;

    // Dedicated LFOs (10 total)
    LFOSettings cutoffLFO;
    LFOSettings resonanceLFO;
    LFOSettings envModLFO;
    LFOSettings decayLFO;
    LFOSettings accentLFO;
    LFOSettings waveformLFO;
    LFOSettings subOscLFO;
    LFOSettings driveLFO;
    LFOSettings volumeLFO;
    LFOSettings delayMixLFO;
};
