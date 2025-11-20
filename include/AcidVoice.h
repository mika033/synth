#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

//==============================================================================
/**
 * Synthesizer voice for Acid bass sounds.
 * Features a sawtooth/square oscillator with resonant filter and envelope.
 */
class AcidVoice : public juce::SynthesiserVoice
{
public:
    AcidVoice();

    bool canPlaySound(juce::SynthesiserSound*) override;

    void startNote(int midiNoteNumber, float velocity,
                   juce::SynthesiserSound*, int currentPitchWheelPosition) override;

    void stopNote(float velocity, bool allowTailOff) override;

    void pitchWheelMoved(int newPitchWheelValue) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                        int startSample, int numSamples) override;

    void setCurrentPlaybackSampleRate(double newRate) override;

    // Main Parameter setters
    void setCutoff(float cutoffHz);
    void setResonance(float resonance);
    void setEnvMod(float envMod);
    void setAccent(float accent);

    // Three oscillators: waveform (0=sine, 0.5=saw, 1=square), coarse (-24 to +24 semitones), fine (-100 to +100 cents), mix (0 to 1)
    void setOscillator1(float wave, int coarse, float fine, float mix);
    void setOscillator2(float wave, int coarse, float fine, float mix);
    void setOscillator3(float wave, int coarse, float fine, float mix);
    void setNoiseMix(float mix);
    void setNoiseType(float type);
    void setNoiseDecay(float decay);

    void setDrive(float drive);
    void setVolume(float volume);
    void setGlobalOctave(int octave);
    void setBPM(double bpm);
    void setFilterFeedback(float feedback);
    void setSaturationType(int type);

    // Analog character setters
    void setDrift(float amount);
    void setPhaseRandom(float amount);
    void setUnison(float amount);

    // ADSR setters
    void setFilterADSR(float attack, float decay, float sustain, float release);
    void setAmpADSR(float attack, float decay, float sustain, float release);

    // Dedicated LFO setters (for each of 10 parameters)
    void setCutoffLFO(int rate, int waveform, float depth);
    void setResonanceLFO(int rate, int waveform, float depth);
    void setEnvModLFO(int rate, int waveform, float depth);
    void setDecayLFO(int rate, int waveform, float depth);
    void setAccentLFO(int rate, int waveform, float depth);
    void setWaveformLFO(int rate, int waveform, float depth);
    void setSubOscLFO(int rate, int waveform, float depth);
    void setDriveLFO(int rate, int waveform, float depth);
    void setVolumeLFO(int rate, int waveform, float depth);
    void setDelayMixLFO(int rate, int waveform, float depth);

private:
    // LFO State structure
    struct LFO
    {
        double phase = 0.0;
        double frequency = 2.0;
        int rate = 6;  // Index 6 = 1/1 (whole note)
        int waveform = 0; // 0=Sine, 1=Triangle, 2=SawUp, 3=SawDown, 4=Square, 5=Random
        float depth = 0.0f;
        float lastRandomValue = 0.0f;
    };
    // Three parallel oscillators
    struct Oscillator
    {
        double angle = 0.0;
        double angleDelta = 0.0;
        double targetAngleDelta = 0.0;
        float wave = 0.5f; // 0=sine, 0.5=saw, 1=square
        int coarseTune = 0; // -24 to +24 semitones
        float fineTune = 0.0f; // -100 to +100 cents
        float mix = 0.0f; // 0 to 1
    };

    Oscillator osc1;
    Oscillator osc2;
    Oscillator osc3;

    // Noise oscillator
    float noiseMix = 0.0f;
    float noiseType = 0.0f; // 0=white, 0.5=pink, 1.0=filtered
    float noiseDecay = 0.0f; // Decay time in seconds
    juce::Random noiseRandom;
    juce::ADSR noiseADSR; // Dedicated envelope for noise decay
    juce::ADSR::Parameters noiseADSRParams;
    double pinkNoiseB0 = 0.0, pinkNoiseB1 = 0.0, pinkNoiseB2 = 0.0; // Pink noise filter state

    // Saturation/Drive
    float driveAmount = 0.0f;
    int saturationType = 0; // 0=Clean, 1=Warm, 2=Tube, 3=Hard, 4=Acid

    // Filter (resonant low-pass)
    double filterCutoff = 1000.0;
    double filterResonance = 0.7;
    float filterFeedback = 0.0f;
    double filter1 = 0.0;
    double filter2 = 0.0;

    // Envelope modulation amount and accent
    float envMod = 0.5f;
    float accentAmount = 0.0f;

    // ADSR for filter envelope
    juce::ADSR filterADSR;
    juce::ADSR::Parameters filterADSRParams;

    // ADSR for amplitude envelope
    juce::ADSR ampADSR;
    juce::ADSR::Parameters ampADSRParams;

    // Playback
    double sampleRate = 44100.0;
    int currentMidiNote = 0;
    float currentVelocity = 0.0f;
    float volumeLevel = 0.7f;
    int globalOctaveShift = 0; // -2 to +2 octave shift
    double currentBPM = 120.0;

    // Analog character parameters
    float driftAmount = 0.0f; // 0 to 1
    float phaseRandomAmount = 0.0f; // 0 to 1
    float unisonAmount = 0.0f; // 0 to 1

    // Drift state (slow random pitch modulation) - per oscillator
    juce::Random driftRandom;
    double driftPhase1 = 0.0;
    double driftPhase2 = 0.0;
    double driftPhase3 = 0.0;
    double driftFrequency = 0.1; // Very slow LFO (0.1 Hz)
    double driftPitchRatio1 = 1.0;
    double driftPitchRatio2 = 1.0;
    double driftPitchRatio3 = 1.0;

    // Unison state (multiple detuned voices)
    static constexpr int maxUnisonVoices = 3;
    double unisonAngles1[maxUnisonVoices] = {0.0, 0.0, 0.0}; // Separate angles per voice
    double unisonAngles2[maxUnisonVoices] = {0.0, 0.0, 0.0};
    double unisonAngles3[maxUnisonVoices] = {0.0, 0.0, 0.0};
    double unisonPhaseOffsets[maxUnisonVoices] = {0.0, juce::MathConstants<double>::pi / 6.0, -juce::MathConstants<double>::pi / 6.0}; // Phase spread
    float unisonDetuneAmounts[maxUnisonVoices] = {0.0f, -1.0f, 1.0f}; // Multiplier for ±10 cents max

    // 10 Dedicated LFOs (one for each parameter)
    LFO cutoffLFO;
    LFO resonanceLFO;
    LFO envModLFO;
    LFO decayLFO;
    LFO accentLFO;
    LFO waveformLFO;
    LFO subOscLFO;
    LFO driveLFO;
    LFO volumeLFO;
    LFO delayMixLFO;

    // Helper functions
    double generateSingleOscillator(Oscillator& osc);
    double generateOscillator();
    void processFilter(double& sample, double cutoffLFOValue, double resonanceLFOValue, float modulatedEnvMod, float filterEnvValue);
    void applySaturation(double& sample);
    void updateAngleDelta();
    double getLFOValue(LFO& lfo);
    void updateLFOFrequency(LFO& lfo);
    void advanceLFO(LFO& lfo);
};

//==============================================================================
/**
 * Simple synth sound class (required by JUCE synthesiser)
 */
class AcidSound : public juce::SynthesiserSound
{
public:
    AcidSound() {}

    bool appliesToNote(int) override { return true; }
    bool appliesToChannel(int) override { return true; }
};
