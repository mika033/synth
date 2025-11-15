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

    // Parameter setters
    void setCutoff(float cutoffHz);
    void setResonance(float resonance);
    void setEnvMod(float envMod);
    void setDecay(float decaySeconds);
    void setAccent(float accent);
    void setWaveform(int waveformType); // 0 = saw, 1 = square

private:
    // Oscillator
    double currentAngle = 0.0;
    double angleDelta = 0.0;
    double targetAngleDelta = 0.0;
    int waveform = 0; // 0 = saw, 1 = square

    // Filter (resonant low-pass)
    double filterCutoff = 1000.0;
    double filterResonance = 0.7;
    double filter1 = 0.0;
    double filter2 = 0.0;

    // Envelope
    float envValue = 0.0f;
    float envDecay = 0.3f;
    float envMod = 0.5f;
    float accentAmount = 0.0f;

    // ADSR for amplitude
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;

    // Playback
    double sampleRate = 44100.0;
    int currentMidiNote = 0;
    float currentVelocity = 0.0f;

    // Helper functions
    double generateOscillator();
    void processFilter(double& sample);
    void updateAngleDelta();
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
