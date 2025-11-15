#include "AcidVoice.h"

AcidVoice::AcidVoice()
{
    // Setup ADSR for amplitude envelope
    adsrParams.attack = 0.001f;   // Fast attack
    adsrParams.decay = 0.3f;       // Medium decay
    adsrParams.sustain = 0.0f;     // No sustain (classic 303 behavior)
    adsrParams.release = 0.1f;     // Short release
    adsr.setParameters(adsrParams);
}

bool AcidVoice::canPlaySound(juce::SynthesiserSound* sound)
{
    return dynamic_cast<AcidSound*>(sound) != nullptr;
}

void AcidVoice::startNote(int midiNoteNumber, float velocity,
                          juce::SynthesiserSound*, int /*currentPitchWheelPosition*/)
{
    currentMidiNote = midiNoteNumber;
    currentVelocity = velocity;

    // Use Accent parameter to scale how much velocity affects filter envelope
    // accentAmount (0-1) controls the intensity of velocity sensitivity
    // velocity affects the filter envelope start value
    float velocityModulation = (velocity - 0.5f) * 2.0f * accentAmount; // -1 to +1 scaled by accent
    envValue = 1.0f + velocityModulation;
    envValue = juce::jlimit(0.1f, 2.0f, envValue); // Clamp to reasonable range

    // Reset filter states to prevent instability and volume fluctuations
    filter1 = 0.0;
    filter2 = 0.0;

    // Update frequency
    updateAngleDelta();

    // Start amplitude ADSR
    adsr.noteOn();
}

void AcidVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    adsr.noteOff();

    if (!allowTailOff || !adsr.isActive())
        clearCurrentNote();
}

void AcidVoice::pitchWheelMoved(int /*newPitchWheelValue*/)
{
}

void AcidVoice::controllerMoved(int /*controllerNumber*/, int /*newControllerValue*/)
{
}

void AcidVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
                                int startSample, int numSamples)
{
    if (!adsr.isActive())
    {
        clearCurrentNote();
        return;
    }

    while (--numSamples >= 0)
    {
        // Generate oscillator sample
        double sample = generateOscillator();

        // Update filter envelope
        envValue *= (1.0f - envDecay * 0.01f);

        // Process through resonant filter
        processFilter(sample);

        // Apply amplitude envelope
        float amplitude = adsr.getNextSample();
        sample *= amplitude;

        // Write to both channels
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample(channel, startSample, static_cast<float>(sample) * 0.3f);

        ++startSample;

        // Smooth portamento
        currentAngle += angleDelta;
        angleDelta += (targetAngleDelta - angleDelta) * 0.0001;

        if (currentAngle > juce::MathConstants<double>::twoPi)
            currentAngle -= juce::MathConstants<double>::twoPi;
    }
}

void AcidVoice::setCurrentPlaybackSampleRate(double newRate)
{
    if (newRate > 0)
    {
        sampleRate = newRate;
        adsr.setSampleRate(newRate);
        updateAngleDelta();
    }
}

void AcidVoice::setCutoff(float cutoffHz)
{
    filterCutoff = juce::jlimit(20.0, 20000.0, static_cast<double>(cutoffHz));
}

void AcidVoice::setResonance(float resonance)
{
    filterResonance = juce::jlimit(0.0, 0.99, static_cast<double>(resonance));
}

void AcidVoice::setEnvMod(float envModAmount)
{
    envMod = juce::jlimit(0.0f, 1.0f, envModAmount);
}

void AcidVoice::setDecay(float decaySeconds)
{
    envDecay = juce::jlimit(0.01f, 1.0f, decaySeconds);
    adsrParams.decay = decaySeconds;
    adsr.setParameters(adsrParams);
}

void AcidVoice::setAccent(float accent)
{
    accentAmount = juce::jlimit(0.0f, 1.0f, accent);
}

void AcidVoice::setWaveform(int waveformType)
{
    waveform = waveformType;
}

double AcidVoice::generateOscillator()
{
    double sample;

    if (waveform == 0)
    {
        // Sawtooth wave
        sample = (currentAngle / juce::MathConstants<double>::pi) - 1.0;
    }
    else
    {
        // Square wave
        sample = currentAngle < juce::MathConstants<double>::pi ? 1.0 : -1.0;
    }

    return sample;
}

void AcidVoice::processFilter(double& sample)
{
    // State-variable filter (resonant low-pass)
    // This gives that classic TB-303 sound

    // Calculate filter frequency with envelope modulation
    double modulation = envValue * envMod * 8000.0;
    double modulatedCutoff = juce::jlimit(20.0, 20000.0, filterCutoff + modulation);

    // Calculate filter coefficients
    double f = 2.0 * std::sin(juce::MathConstants<double>::pi * modulatedCutoff / sampleRate);
    f = juce::jlimit(0.0, 1.0, f);

    // State-variable filter implementation
    // Invert resonance: higher filterResonance = less damping = more resonance
    // Clamp damping to minimum value to prevent total instability
    double damping = juce::jlimit(0.05, 1.0, 1.0 - filterResonance);

    double lowpass = filter2 + f * filter1;
    double highpass = sample - lowpass - damping * filter1;
    double bandpass = f * highpass + filter1;

    // Clamp filter states to prevent runaway values
    filter1 = juce::jlimit(-10.0, 10.0, bandpass);
    filter2 = juce::jlimit(-10.0, 10.0, lowpass);

    // Output low-pass filtered signal
    sample = lowpass;
}

void AcidVoice::updateAngleDelta()
{
    double cyclesPerSecond = juce::MidiMessage::getMidiNoteInHertz(currentMidiNote);
    targetAngleDelta = cyclesPerSecond * juce::MathConstants<double>::twoPi / sampleRate;
}
