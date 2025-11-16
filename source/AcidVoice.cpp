#include "AcidVoice.h"

AcidVoice::AcidVoice()
{
    // Setup ADSR for amplitude envelope
    adsrParams.attack = 0.003f;   // Fast attack (3ms to prevent clicks)
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
        // Get all 10 LFO modulation values (-1 to +1)
        double cutoffLFOValue = getLFOValue(cutoffLFO);
        double resonanceLFOValue = getLFOValue(resonanceLFO);
        double envModLFOValue = getLFOValue(envModLFO);
        double decayLFOValue = getLFOValue(decayLFO);
        double accentLFOValue = getLFOValue(accentLFO);
        double waveformLFOValue = getLFOValue(waveformLFO);
        double subOscLFOValue = getLFOValue(subOscLFO);
        double driveLFOValue = getLFOValue(driveLFO);
        double volumeLFOValue = getLFOValue(volumeLFO);
        // delayMixLFO is used in the processor's delay effect, not here

        // Apply waveform LFO modulation
        float modulatedWaveform = waveformMorph + static_cast<float>(waveformLFOValue) * waveformLFO.depth;
        modulatedWaveform = juce::jlimit(0.0f, 1.0f, modulatedWaveform);

        // Temporarily set the waveform for this sample
        float originalWaveform = waveformMorph;
        waveformMorph = modulatedWaveform;

        // Generate main oscillator sample
        double sample = generateOscillator();

        // Restore original waveform
        waveformMorph = originalWaveform;

        // Apply sub-oscillator LFO modulation
        float modulatedSubOsc = subOscMix + static_cast<float>(subOscLFOValue) * subOscLFO.depth;
        modulatedSubOsc = juce::jlimit(0.0f, 1.0f, modulatedSubOsc);

        // Add sub-oscillator (one octave down)
        double subSample = generateSubOscillator();
        sample = sample * (1.0f - modulatedSubOsc) + subSample * modulatedSubOsc;

        // Apply envelope mod LFO modulation
        float modulatedEnvMod = envMod + static_cast<float>(envModLFOValue) * envModLFO.depth;
        modulatedEnvMod = juce::jlimit(0.0f, 1.0f, modulatedEnvMod);

        // Apply decay LFO modulation
        float modulatedDecay = envDecay + static_cast<float>(decayLFOValue) * decayLFO.depth * 0.5f;
        modulatedDecay = juce::jlimit(0.0f, 1.0f, modulatedDecay);

        // Update filter envelope with modulated decay
        envValue *= (1.0f - modulatedDecay * 0.01f);

        // Apply accent LFO modulation to envelope value for rhythmic filter movement
        float accentModulation = 1.0f + static_cast<float>(accentLFOValue) * accentLFO.depth * 0.5f;
        float modulatedEnvValue = envValue * accentModulation;

        // Store original envValue
        float originalEnvValue = envValue;
        envValue = modulatedEnvValue;

        // Process through resonant filter (with dedicated cutoff and resonance LFOs)
        processFilter(sample, cutoffLFOValue, resonanceLFOValue, modulatedEnvMod);

        // Restore original envValue
        envValue = originalEnvValue;

        // Apply drive LFO modulation
        float modulatedDrive = driveAmount + static_cast<float>(driveLFOValue) * driveLFO.depth;
        modulatedDrive = juce::jlimit(0.0f, 1.0f, modulatedDrive);

        // Temporarily set the drive for this sample
        float originalDrive = driveAmount;
        driveAmount = modulatedDrive;

        // Apply saturation/drive
        applySaturation(sample);

        // Restore original drive
        driveAmount = originalDrive;

        // Apply amplitude envelope
        float amplitude = adsr.getNextSample();
        sample *= amplitude;

        // Apply volume LFO modulation
        float modulatedVolume = volumeLevel + static_cast<float>(volumeLFOValue) * volumeLFO.depth * 0.5f;
        modulatedVolume = juce::jlimit(0.0f, 1.5f, modulatedVolume);

        sample *= modulatedVolume;

        // Write to both channels
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.addSample(channel, startSample, static_cast<float>(sample));

        ++startSample;

        // Advance oscillators (no portamento - instant pitch changes)
        currentAngle += angleDelta;
        subAngle += angleDelta * 0.5; // Sub is one octave down
        angleDelta = targetAngleDelta; // Instant pitch change (no slide)

        if (currentAngle > juce::MathConstants<double>::twoPi)
            currentAngle -= juce::MathConstants<double>::twoPi;
        if (subAngle > juce::MathConstants<double>::twoPi)
            subAngle -= juce::MathConstants<double>::twoPi;

        // Advance all 10 LFO phases
        advanceLFO(cutoffLFO);
        advanceLFO(resonanceLFO);
        advanceLFO(envModLFO);
        advanceLFO(decayLFO);
        advanceLFO(accentLFO);
        advanceLFO(waveformLFO);
        advanceLFO(subOscLFO);
        advanceLFO(driveLFO);
        advanceLFO(volumeLFO);
        advanceLFO(delayMixLFO);
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

void AcidVoice::setWaveform(float morph)
{
    waveformMorph = juce::jlimit(0.0f, 1.0f, morph);
}

void AcidVoice::setSubOscMix(float mix)
{
    subOscMix = juce::jlimit(0.0f, 1.0f, mix);
}

void AcidVoice::setDrive(float drive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, drive);
}

void AcidVoice::setVolume(float volume)
{
    volumeLevel = juce::jlimit(0.0f, 1.0f, volume);
}

void AcidVoice::setBPM(double bpm)
{
    currentBPM = juce::jlimit(20.0, 999.0, bpm);
    // Update all LFO frequencies when BPM changes
    updateLFOFrequency(cutoffLFO);
    updateLFOFrequency(resonanceLFO);
    updateLFOFrequency(envModLFO);
    updateLFOFrequency(decayLFO);
    updateLFOFrequency(accentLFO);
    updateLFOFrequency(waveformLFO);
    updateLFOFrequency(subOscLFO);
    updateLFOFrequency(driveLFO);
    updateLFOFrequency(volumeLFO);
    updateLFOFrequency(delayMixLFO);
}

void AcidVoice::setFilterFeedback(float feedback)
{
    filterFeedback = juce::jlimit(0.0f, 1.0f, feedback);
}

void AcidVoice::setSaturationType(int type)
{
    saturationType = juce::jlimit(0, 4, type); // 0=Clean, 1=Warm, 2=Tube, 3=Hard, 4=Acid
}

// Dedicated LFO setters
void AcidVoice::setCutoffLFO(int rate, int waveform, float depth)
{
    cutoffLFO.rate = juce::jlimit(0, 14, rate);
    cutoffLFO.waveform = juce::jlimit(0, 5, waveform);
    cutoffLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(cutoffLFO);
}

void AcidVoice::setResonanceLFO(int rate, int waveform, float depth)
{
    resonanceLFO.rate = juce::jlimit(0, 14, rate);
    resonanceLFO.waveform = juce::jlimit(0, 5, waveform);
    resonanceLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(resonanceLFO);
}

void AcidVoice::setEnvModLFO(int rate, int waveform, float depth)
{
    envModLFO.rate = juce::jlimit(0, 14, rate);
    envModLFO.waveform = juce::jlimit(0, 5, waveform);
    envModLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(envModLFO);
}

void AcidVoice::setDecayLFO(int rate, int waveform, float depth)
{
    decayLFO.rate = juce::jlimit(0, 14, rate);
    decayLFO.waveform = juce::jlimit(0, 5, waveform);
    decayLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(decayLFO);
}

void AcidVoice::setAccentLFO(int rate, int waveform, float depth)
{
    accentLFO.rate = juce::jlimit(0, 14, rate);
    accentLFO.waveform = juce::jlimit(0, 5, waveform);
    accentLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(accentLFO);
}

void AcidVoice::setWaveformLFO(int rate, int waveform, float depth)
{
    waveformLFO.rate = juce::jlimit(0, 14, rate);
    waveformLFO.waveform = juce::jlimit(0, 5, waveform);
    waveformLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(waveformLFO);
}

void AcidVoice::setSubOscLFO(int rate, int waveform, float depth)
{
    subOscLFO.rate = juce::jlimit(0, 14, rate);
    subOscLFO.waveform = juce::jlimit(0, 5, waveform);
    subOscLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(subOscLFO);
}

void AcidVoice::setDriveLFO(int rate, int waveform, float depth)
{
    driveLFO.rate = juce::jlimit(0, 14, rate);
    driveLFO.waveform = juce::jlimit(0, 5, waveform);
    driveLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(driveLFO);
}

void AcidVoice::setVolumeLFO(int rate, int waveform, float depth)
{
    volumeLFO.rate = juce::jlimit(0, 14, rate);
    volumeLFO.waveform = juce::jlimit(0, 5, waveform);
    volumeLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(volumeLFO);
}

void AcidVoice::setDelayMixLFO(int rate, int waveform, float depth)
{
    delayMixLFO.rate = juce::jlimit(0, 14, rate);
    delayMixLFO.waveform = juce::jlimit(0, 5, waveform);
    delayMixLFO.depth = juce::jlimit(0.0f, 1.0f, depth);
    updateLFOFrequency(delayMixLFO);
}

double AcidVoice::generateOscillator()
{
    // Generate both waveforms
    double sawtoothSample = (currentAngle / juce::MathConstants<double>::pi) - 1.0;
    double squareSample = currentAngle < juce::MathConstants<double>::pi ? 1.0 : -1.0;

    // Morph between them based on waveformMorph (0.0 = saw, 1.0 = square)
    double sample = sawtoothSample * (1.0 - waveformMorph) + squareSample * waveformMorph;

    return sample;
}

double AcidVoice::generateSubOscillator()
{
    // Sub-oscillator is always a sine wave for maximum low-end
    // One octave below the main oscillator
    return std::sin(subAngle);
}

void AcidVoice::applySaturation(double& sample)
{
    // Soft clipping saturation
    // More drive = more harmonic content and aggression
    if (driveAmount > 0.001f)
    {
        // Pre-gain based on drive amount
        double gain = 1.0 + driveAmount * 4.0; // Up to 5x gain
        sample *= gain;

        // Soft clipping using tanh
        // tanh naturally compresses signals > 1.0 and adds harmonics
        sample = std::tanh(sample);

        // Compensate for volume loss
        sample *= (1.0 + driveAmount * 0.5);
    }
}

void AcidVoice::processFilter(double& sample, double cutoffLFOValue, double resonanceLFOValue, float modulatedEnvMod)
{
    // State-variable filter (resonant low-pass)
    // This gives that classic TB-303 sound

    // Calculate filter frequency with envelope modulation
    double modulation = envValue * modulatedEnvMod * 8000.0;

    // Add dedicated cutoff LFO modulation
    modulation += cutoffLFOValue * cutoffLFO.depth * 3000.0; // LFO can modulate +/- 3kHz

    double modulatedCutoff = juce::jlimit(20.0, 20000.0, filterCutoff + modulation);

    // Calculate filter coefficients
    double f = 2.0 * std::sin(juce::MathConstants<double>::pi * modulatedCutoff / sampleRate);
    f = juce::jlimit(0.0, 1.0, f);

    // State-variable filter implementation
    // Apply dedicated resonance LFO modulation
    // Note: Negate LFO value because resonance gets inverted later (1.0 - modulatedResonance)
    double modulatedResonance = filterResonance - resonanceLFOValue * resonanceLFO.depth * 0.3;
    modulatedResonance = juce::jlimit(0.0, 0.99, modulatedResonance);

    // Invert resonance: higher filterResonance = less damping = more resonance
    // Clamp damping to minimum value to prevent total instability
    double damping = juce::jlimit(0.05, 1.0, 1.0 - modulatedResonance);

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

void AcidVoice::updateLFOFrequency(LFO& lfo)
{
    // Calculate LFO frequency based on tempo and rate division
    double beatsPerSecond = currentBPM / 60.0;

    // Rate divisions (15 options):
    // 0=1/16, 1=1/8, 2=1/4, 3=1/3, 4=1/2, 5=3/4, 6=1/1, 7=3/2, 8=2/1, 9=3/1, 10=4/1, 11=6/1, 12=8/1, 13=12/1, 14=16/1
    const double divisions[] = {
        4.0,     // 1/16
        2.0,     // 1/8
        1.0,     // 1/4
        1.333,   // 1/3
        0.5,     // 1/2
        0.333,   // 3/4
        0.25,    // 1/1 (whole note)
        0.1667,  // 3/2 (dotted whole note = 1.5 bars)
        0.125,   // 2/1 (two whole notes)
        0.0833,  // 3/1
        0.0625,  // 4/1
        0.0417,  // 6/1
        0.03125, // 8/1
        0.0208,  // 12/1
        0.015625 // 16/1
    };

    double notesPerBeat = divisions[lfo.rate];
    lfo.frequency = beatsPerSecond * notesPerBeat;
}

double AcidVoice::getLFOValue(LFO& lfo)
{
    // Generate LFO waveform based on type
    // Output range: -1 to +1

    switch (lfo.waveform)
    {
        case 0: // Sine
            return std::sin(lfo.phase);

        case 1: // Triangle
        {
            double t = lfo.phase / juce::MathConstants<double>::twoPi;
            return 4.0 * std::abs(t - 0.5) - 1.0;
        }

        case 2: // Saw Up
            return (lfo.phase / juce::MathConstants<double>::pi) - 1.0;

        case 3: // Saw Down
            return 1.0 - (lfo.phase / juce::MathConstants<double>::pi);

        case 4: // Square
            return lfo.phase < juce::MathConstants<double>::pi ? 1.0 : -1.0;

        case 5: // Random (sample & hold)
        {
            // Update random value when phase wraps
            if (lfo.phase < juce::MathConstants<double>::twoPi / sampleRate)
            {
                lfo.lastRandomValue = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
            }
            return lfo.lastRandomValue;
        }

        default:
            return std::sin(lfo.phase);
    }
}

void AcidVoice::advanceLFO(LFO& lfo)
{
    lfo.phase += juce::MathConstants<double>::twoPi * lfo.frequency / sampleRate;
    if (lfo.phase >= juce::MathConstants<double>::twoPi)
        lfo.phase -= juce::MathConstants<double>::twoPi;
}
