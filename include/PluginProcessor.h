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

    //==============================================================================
    // Playback control (starts/stops arp and sequencer)
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return isPlaybackActive; }

private:
    //==============================================================================
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState parameters;

    // Main Parameter IDs
    static constexpr const char* CUTOFF_ID = "cutoff";
    static constexpr const char* RESONANCE_ID = "resonance";
    static constexpr const char* ENV_MOD_ID = "envmod";
    static constexpr const char* ACCENT_ID = "accent";
    static constexpr const char* WAVEFORM_ID = "waveform";
    static constexpr const char* SUB_OSC_ID = "subosc";
    static constexpr const char* DRIVE_ID = "drive";
    static constexpr const char* VOLUME_ID = "volume";
    static constexpr const char* DELAY_TIME_ID = "delaytime";
    static constexpr const char* DELAY_FEEDBACK_ID = "delayfeedback";
    static constexpr const char* DELAY_MIX_ID = "delaymix";

    // Filter & Saturation Enhancement Parameters
    static constexpr const char* FILTER_FEEDBACK_ID = "filterfeedback";
    static constexpr const char* SATURATION_TYPE_ID = "saturationtype";

    // Filter ADSR Parameters
    static constexpr const char* FILTER_ATTACK_ID = "filterattack";
    static constexpr const char* FILTER_DECAY_ID = "filterdecay";
    static constexpr const char* FILTER_SUSTAIN_ID = "filtersustain";
    static constexpr const char* FILTER_RELEASE_ID = "filterrelease";

    // Amplitude ADSR Parameters
    static constexpr const char* AMP_ATTACK_ID = "ampattack";
    static constexpr const char* AMP_DECAY_ID = "ampdecay";
    static constexpr const char* AMP_SUSTAIN_ID = "ampsustain";
    static constexpr const char* AMP_RELEASE_ID = "amprelease";

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
    static constexpr const char* ARP_OCTAVE_SHIFT_ID = "arpoctaveshift";
    static constexpr const char* ARP_SWING_ID = "arpswing";

    // Sequencer Parameter IDs
    static constexpr const char* SEQ_ENABLED_ID = "seqenabled";
    static constexpr const char* SEQ_ROOT_ID = "seqroot";
    static constexpr const char* SEQ_SCALE_ID = "seqscale";

    // Progression Parameter IDs
    static constexpr const char* PROG_ENABLED_ID = "progenabled";
    static constexpr const char* PROG_STEPS_ID = "progsteps";
    static constexpr const char* PROG_LENGTH_ID = "proglength";
    static constexpr const char* PROG_STEP1_ID = "progstep1";
    static constexpr const char* PROG_STEP2_ID = "progstep2";
    static constexpr const char* PROG_STEP3_ID = "progstep3";
    static constexpr const char* PROG_STEP4_ID = "progstep4";
    static constexpr const char* PROG_STEP5_ID = "progstep5";
    static constexpr const char* PROG_STEP6_ID = "progstep6";
    static constexpr const char* PROG_STEP7_ID = "progstep7";
    static constexpr const char* PROG_STEP8_ID = "progstep8";

    // Global Parameters
    static constexpr const char* GLOBAL_BPM_ID = "globalbpm";
    static constexpr const char* MASTER_VOLUME_ID = "mastervolume";

public:
    // Sequencer state (public for UI access)
    static constexpr int NUM_SEQ_STEPS = 16;
    static constexpr int NUM_SCALE_DEGREES = 8;
    int sequencerPattern[NUM_SEQ_STEPS]; // Stores scale degree for each step (-1 = no note)
    int currentSeqStep = 0;

    // Progression state (public for UI access)
    static constexpr int NUM_PROGRESSION_STEPS = 8;
    int currentProgressionStep = 0;

private:
    // Parameter update
    void updateVoiceParameters();

    // Preset management
    void loadPreset(int presetIndex);

    // Delay effect
    juce::dsp::DelayLine<float> delayLine { 192000 }; // Max 4 seconds at 48kHz
    std::vector<float> delayBuffer;
    double currentSampleRate = 44100.0;
    double currentBPM = 120.0;

    // Delay Mix LFO state
    struct DelayMixLFO {
        double phase = 0.0;
        double frequency = 2.0;
        int rate = 6;  // Index 6 = 1/1
        int waveform = 0; // 0=Sine
        float depth = 0.0f;
        float lastRandomValue = 0.0f;
    };
    DelayMixLFO delayMixLFO;

    // Arpeggiator state
    std::vector<int> heldNotes; // Currently held MIDI notes
    int currentArpNote = 0;      // Index into heldNotes
    double arpStepTime = 0.0;    // Time since last arp step (in samples)
    double lastNoteOffTime = 0.0; // Time for note-off based on gate length
    int lastPlayedNote = -1;     // Last arpeggio note that was triggered
    bool isNoteCurrentlyOn = false; // Track if we're in note-on phase
    int arpStepCounter = 0;      // Counter for swing (even/odd steps)
    int lastProgressionStepForArp = -1; // Track progression step to detect changes

    // Playback state - controls whether arp/sequencer are active
    bool isPlaybackActive = false;
    double totalPlaybackTime = 0.0; // Track total time for bar synchronization

    // Arpeggiator helper functions
    void processArpeggiator(juce::MidiBuffer& midiMessages, int numSamples);
    double getArpStepLengthInSamples() const;
    int getNextArpNote();

    // Sequencer state (private timing/control variables)
    double seqStepTime = 0.0;
    double lastSeqNoteOffTime = 0.0;
    int lastSeqPlayedNote = -1;
    bool isSeqNoteCurrentlyOn = false;

    // Sequencer helper functions
    void processSequencer(juce::MidiBuffer& midiMessages, int numSamples);
    double getSeqStepLengthInSamples() const;
    int getSequencerNote(int step);
    int scaleDegreesToMidiNote(int scaleDegree, int rootNote, int scaleType);

    // Progression state and helpers
    double progressionBarTime = 0.0;
    bool progressionWasEnabled = false;
    bool progressionSyncedToBar = false;
    void updateProgressionStep(int numSamples);
    int getCurrentProgressionOffset() const;

    // Delay Mix LFO helper functions
    void updateDelayMixLFO();
    double getDelayMixLFOValue();
    void advanceDelayMixLFO();

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
