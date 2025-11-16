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
    void setWaveform(float waveformMorph); // 0.0 = saw, 1.0 = square, morph in between
    void setSubOscMix(float mix);
    void setDrive(float drive);
    void setVolume(float volume);
    void setBPM(double bpm);
    void setFilterFeedback(float feedback);
    void setSaturationType(int type);

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
    // Main Oscillator
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double targetAngleDelta = 0.0;
    float waveformMorph = 0.0f; // 0.0 = saw, 1.0 = square, morph in between

    // Sub-oscillator (one octave down)
    double subAngle = 0.0;
    float subOscMix = 0.5f;

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
    double currentBPM = 120.0;

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
    double generateOscillator();
    double generateSubOscillator();
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
