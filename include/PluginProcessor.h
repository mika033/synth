#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include "AcidVoice.h"

// Forward declaration
class SnorkelSynthAudioProcessorEditor;

//==============================================================================
/**
 * Main audio processor for Snorkel Synth VST plugin
 */
class SnorkelSynthAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    SnorkelSynthAudioProcessor();
    ~SnorkelSynthAudioProcessor() override;

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
    // JSON Preset Management
    void loadPresetsFromJSON();
    void saveSynthPresetToJSON(const juce::String& presetName);
    void saveSequencerPresetToJSON(const juce::String& presetName);
    juce::StringArray getSynthPresetNames() const;
    juce::StringArray getSequencerPresetNames() const;

    // Public access to JSON data for UI components
    juce::var synthPresetsJSON;  // Combined system + user presets for UI
    juce::var sequencerPresetsJSON;
    juce::var randomizationConfigJSON;
    int numSystemSynthPresets = 0;  // Track count of system presets for divider
    int numSystemSequencerPresets = 0;  // Track count of system sequencer presets for divider

    //==============================================================================
    // Playback control (starts/stops arp and sequencer)
    void startPlayback();
    void stopPlayback();
    bool isPlaying() const { return isPlaybackActive; }

    //==============================================================================
    // Editor messaging
    void setEditor(SnorkelSynthAudioProcessorEditor* editor) { currentEditor = editor; }
    void showEditorMessage(const juce::String& message);

private:
    //==============================================================================
    juce::Synthesiser synth;
    juce::AudioProcessorValueTreeState parameters;
    SnorkelSynthAudioProcessorEditor* currentEditor = nullptr;

    // Main Parameter IDs
    static constexpr const char* CUTOFF_ID = "cutoff";
    static constexpr const char* RESONANCE_ID = "resonance";
    static constexpr const char* ENV_MOD_ID = "envmod";
    static constexpr const char* ACCENT_ID = "accent";

    // Three parallel oscillators
    static constexpr const char* OSC1_WAVE_ID = "osc1wave";
    static constexpr const char* OSC1_COARSE_ID = "osc1coarse";
    static constexpr const char* OSC1_FINE_ID = "osc1fine";
    static constexpr const char* OSC1_MIX_ID = "osc1mix";

    static constexpr const char* OSC2_WAVE_ID = "osc2wave";
    static constexpr const char* OSC2_COARSE_ID = "osc2coarse";
    static constexpr const char* OSC2_FINE_ID = "osc2fine";
    static constexpr const char* OSC2_MIX_ID = "osc2mix";

    static constexpr const char* OSC3_WAVE_ID = "osc3wave";
    static constexpr const char* OSC3_COARSE_ID = "osc3coarse";
    static constexpr const char* OSC3_FINE_ID = "osc3fine";
    static constexpr const char* OSC3_MIX_ID = "osc3mix";

    static constexpr const char* NOISE_TYPE_ID = "noisetype";
    static constexpr const char* NOISE_DECAY_ID = "noisedecay";
    static constexpr const char* NOISE_MIX_ID = "noisemix";

    static constexpr const char* DRIVE_ID = "drive";
    static constexpr const char* VOLUME_ID = "volume";
    static constexpr const char* GLOBAL_OCTAVE_ID = "globaloctave";

    // Analog character parameters
    static constexpr const char* DRIFT_ID = "drift";
    static constexpr const char* PHASE_RANDOM_ID = "phaserandom";
    static constexpr const char* UNISON_ID = "unison";

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
    static constexpr const char* SEQ_STEPS_ID = "seqsteps";
    static constexpr const char* SEQ_RATE_ID = "seqrate";

    // Sequencer per-step octave (16 steps)
    static constexpr const char* SEQ_OCTAVE1_ID = "seqoctave1";
    static constexpr const char* SEQ_OCTAVE2_ID = "seqoctave2";
    static constexpr const char* SEQ_OCTAVE3_ID = "seqoctave3";
    static constexpr const char* SEQ_OCTAVE4_ID = "seqoctave4";
    static constexpr const char* SEQ_OCTAVE5_ID = "seqoctave5";
    static constexpr const char* SEQ_OCTAVE6_ID = "seqoctave6";
    static constexpr const char* SEQ_OCTAVE7_ID = "seqoctave7";
    static constexpr const char* SEQ_OCTAVE8_ID = "seqoctave8";
    static constexpr const char* SEQ_OCTAVE9_ID = "seqoctave9";
    static constexpr const char* SEQ_OCTAVE10_ID = "seqoctave10";
    static constexpr const char* SEQ_OCTAVE11_ID = "seqoctave11";
    static constexpr const char* SEQ_OCTAVE12_ID = "seqoctave12";
    static constexpr const char* SEQ_OCTAVE13_ID = "seqoctave13";
    static constexpr const char* SEQ_OCTAVE14_ID = "seqoctave14";
    static constexpr const char* SEQ_OCTAVE15_ID = "seqoctave15";
    static constexpr const char* SEQ_OCTAVE16_ID = "seqoctave16";

    // Sequencer per-step cutoff modulation (16 steps)
    static constexpr const char* SEQ_CUTOFF1_ID = "seqcutoff1";
    static constexpr const char* SEQ_CUTOFF2_ID = "seqcutoff2";
    static constexpr const char* SEQ_CUTOFF3_ID = "seqcutoff3";
    static constexpr const char* SEQ_CUTOFF4_ID = "seqcutoff4";
    static constexpr const char* SEQ_CUTOFF5_ID = "seqcutoff5";
    static constexpr const char* SEQ_CUTOFF6_ID = "seqcutoff6";
    static constexpr const char* SEQ_CUTOFF7_ID = "seqcutoff7";
    static constexpr const char* SEQ_CUTOFF8_ID = "seqcutoff8";
    static constexpr const char* SEQ_CUTOFF9_ID = "seqcutoff9";
    static constexpr const char* SEQ_CUTOFF10_ID = "seqcutoff10";
    static constexpr const char* SEQ_CUTOFF11_ID = "seqcutoff11";
    static constexpr const char* SEQ_CUTOFF12_ID = "seqcutoff12";
    static constexpr const char* SEQ_CUTOFF13_ID = "seqcutoff13";
    static constexpr const char* SEQ_CUTOFF14_ID = "seqcutoff14";
    static constexpr const char* SEQ_CUTOFF15_ID = "seqcutoff15";
    static constexpr const char* SEQ_CUTOFF16_ID = "seqcutoff16";

    // Sequencer accent control amounts
    static constexpr const char* SEQ_ACCENT_VOL_ID = "seqaccentvol";
    static constexpr const char* SEQ_ACCENT_CUTOFF_ID = "seqaccentcutoff";
    static constexpr const char* SEQ_ACCENT_RES_ID = "seqaccentres";
    static constexpr const char* SEQ_ACCENT_DECAY_ID = "seqaccentdecay";
    static constexpr const char* SEQ_ACCENT_DRIVE_ID = "seqaccentdrive";

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
    static constexpr const char* PROG_STEP9_ID = "progstep9";
    static constexpr const char* PROG_STEP10_ID = "progstep10";
    static constexpr const char* PROG_STEP11_ID = "progstep11";
    static constexpr const char* PROG_STEP12_ID = "progstep12";
    static constexpr const char* PROG_STEP13_ID = "progstep13";
    static constexpr const char* PROG_STEP14_ID = "progstep14";
    static constexpr const char* PROG_STEP15_ID = "progstep15";
    static constexpr const char* PROG_STEP16_ID = "progstep16";

    // Global Parameters
    static constexpr const char* GLOBAL_BPM_ID = "globalbpm";
    static constexpr const char* MASTER_VOLUME_ID = "mastervolume";

public:
    // Sequencer state (public for UI access)
    static constexpr int NUM_SEQ_STEPS = 16;
    static constexpr int NUM_SCALE_DEGREES = 8;
    uint8_t sequencerPattern[NUM_SEQ_STEPS]; // Bitmask: bit N = degree N is active (0 = no notes)
    int8_t sequencerOctave[NUM_SEQ_STEPS][NUM_SCALE_DEGREES]; // Per-note octave offset (-2 to +2)
    int currentSeqStep = 0;
    float getCurrentSeqCutoffMod() const; // Get cutoff modulation for current sequencer step

    // Progression state (public for UI access)
    static constexpr int NUM_PROGRESSION_STEPS = 16;
    int currentProgressionStep = 0;

    // Drum machine state (public for UI access)
    static constexpr int NUM_DRUM_STEPS = 16;
    static constexpr int NUM_DRUM_LANES = 4; // Kick, Snare, CHat, OHat
    int drumPattern[NUM_DRUM_LANES][NUM_DRUM_STEPS] = {{0}}; // 0 = off, 1 = on
    int currentDrumStep = 0;
    float sidechainEnvelope = 0.0f; // Current sidechain ducking amount (0-1)
    float getSidechainEnvelope() const { return sidechainEnvelope; }

private:
    // Parameter update
    void updateVoiceParameters();

    // Preset management
    void loadPresetFromJSON(int presetIndex);
    juce::File getDataDirectory() const;
    juce::String formatJSON(const juce::var& json, int indentLevel = 0) const;

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
    float seqAccentDecayMod = 0.0f; // Current accent decay modulation

    // Sequencer helper functions
    void processSequencer(juce::MidiBuffer& midiMessages, int numSamples);
    double getSeqStepLengthInSamples() const;
    std::vector<int> getSequencerNotes(int step); // Returns all active MIDI notes for a step
    int scaleDegreesToMidiNote(int scaleDegree, int rootNote, int scaleType);
    std::vector<int> lastSeqPlayedNotes; // Track multiple notes for note-off

    // Progression state and helpers
    double progressionBarTime = 0.0;
    bool progressionWasEnabled = false;
    bool progressionSyncedToBar = false;
    void updateProgressionStep(int numSamples);
    int getCurrentProgressionOffset() const;

    // Drum machine state and helpers
    double drumStepTime = 0.0;
    void processDrums(int numSamples);

    // Delay Mix LFO helper functions
    void updateDelayMixLFO();
    double getDelayMixLFOValue();
    void advanceDelayMixLFO();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnorkelSynthAudioProcessor)
};

// LFO settings structure
struct LFOSettings
{
    int rate;      // Index into rate options
    int waveform;  // 0=Sine, 1=Triangle, 2=SawUp, 3=SawDown, 4=Square, 5=Random
    float depth;   // 0.0-1.0
};
