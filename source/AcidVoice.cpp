#include "AcidVoice.h"

AcidVoice::AcidVoice()
{
    // Setup ADSR for amplitude envelope (default values)
    ampADSRParams.attack = 0.003f;   // Fast attack (3ms to prevent clicks)
    ampADSRParams.decay = 0.3f;       // Medium decay
    ampADSRParams.sustain = 0.0f;     // No sustain (classic 303 behavior)
    ampADSRParams.release = 0.1f;     // Short release
    ampADSR.setParameters(ampADSRParams);

    // Setup ADSR for filter envelope (default values)
    filterADSRParams.attack = 0.003f;
    filterADSRParams.decay = 0.3f;
    filterADSRParams.sustain = 0.0f;
    filterADSRParams.release = 0.1f;
    filterADSR.setParameters(filterADSRParams);
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

    // Reset filter states to prevent instability and volume fluctuations
    filter1 = 0.0;
    filter2 = 0.0;

    // Update frequency
    updateAngleDelta();

    // Start both ADSRs
    ampADSR.noteOn();
    filterADSR.noteOn();
}

void AcidVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    ampADSR.noteOff();
    filterADSR.noteOff();

    if (!allowTailOff || !ampADSR.isActive())
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
    if (!ampADSR.isActive())
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

        // Get filter ADSR envelope value
        float filterEnvValue = filterADSR.getNextSample();

        // Apply accent LFO modulation to filter envelope for rhythmic filter movement
        float accentModulation = 1.0f + static_cast<float>(accentLFOValue) * accentLFO.depth * 0.5f;
        filterEnvValue *= accentModulation;

        // Apply envelope mod LFO modulation
        float modulatedEnvMod = envMod + static_cast<float>(envModLFOValue) * envModLFO.depth;
        modulatedEnvMod = juce::jlimit(0.0f, 1.0f, modulatedEnvMod);

        // Apply decay LFO to modulate filter envelope decay (for compatibility with existing presets)
        float decayModulation = 1.0f + static_cast<float>(decayLFOValue) * decayLFO.depth;
        float modulatedFilterEnv = filterEnvValue * decayModulation;
        modulatedFilterEnv = juce::jlimit(0.0f, 2.0f, modulatedFilterEnv);

        // Process through resonant filter (with dedicated cutoff and resonance LFOs)
        // Pass filter envelope value to processFilter
        processFilter(sample, cutoffLFOValue, resonanceLFOValue, modulatedEnvMod, modulatedFilterEnv);

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
        float amplitude = ampADSR.getNextSample();
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
        ampADSR.setSampleRate(newRate);
        filterADSR.setSampleRate(newRate);
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

// ADSR setters
void AcidVoice::setFilterADSR(float attack, float decay, float sustain, float release)
{
    filterADSRParams.attack = juce::jlimit(0.0f, 10.0f, attack);
    filterADSRParams.decay = juce::jlimit(0.0f, 10.0f, decay);
    filterADSRParams.sustain = juce::jlimit(0.0f, 1.0f, sustain);
    filterADSRParams.release = juce::jlimit(0.0f, 10.0f, release);
    filterADSR.setParameters(filterADSRParams);
}

void AcidVoice::setAmpADSR(float attack, float decay, float sustain, float release)
{
    ampADSRParams.attack = juce::jlimit(0.0f, 10.0f, attack);
    ampADSRParams.decay = juce::jlimit(0.0f, 10.0f, decay);
    ampADSRParams.sustain = juce::jlimit(0.0f, 1.0f, sustain);
    ampADSRParams.release = juce::jlimit(0.0f, 10.0f, release);
    ampADSR.setParameters(ampADSRParams);
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
    if (driveAmount < 0.001f)
        return;

    // Pre-gain based on drive amount
    double gain = 1.0 + driveAmount * 4.0; // Up to 5x gain
    sample *= gain;

    // Apply different saturation algorithms based on type
    switch (saturationType)
    {
        case 0: // Clean - Soft tanh clipping
            sample = std::tanh(sample);
            sample *= (1.0 + driveAmount * 0.5);
            break;

        case 1: // Warm - Soft asymmetric clipping (even harmonics)
            {
                double absVal = std::abs(sample);
                double sign = (sample >= 0.0) ? 1.0 : -1.0;
                // Asymmetric curve adds even harmonics for warmth
                sample = sign * (absVal / (1.0 + absVal * 0.7));
                sample *= (1.0 + driveAmount * 0.7);
            }
            break;

        case 2: // Tube - Soft saturation with compression
            {
                // Tube-style saturation with soft knee
                double absVal = std::abs(sample);
                if (absVal < 0.5)
                    sample = sample;
                else if (absVal < 1.0)
                    sample = sample * (1.0 - (absVal - 0.5) * 0.5);
                else
                    sample = sample / absVal * 0.75; // Soft limit
                sample *= (1.0 + driveAmount * 0.8);
            }
            break;

        case 3: // Hard - Hard clipping for aggressive sound
            {
                double threshold = 0.8;
                if (sample > threshold)
                    sample = threshold;
                else if (sample < -threshold)
                    sample = -threshold;
                sample *= (1.0 + driveAmount * 1.0);
            }
            break;

        case 4: // Acid - Asymmetric hard clip + bit crushing effect
            {
                // Hard asymmetric clipping
                if (sample > 0.7)
                    sample = 0.7;
                else if (sample < -0.9)
                    sample = -0.9;
                // Add slight bit crushing for digital grit
                double bits = 12.0; // 12-bit depth
                double step = 2.0 / std::pow(2.0, bits);
                sample = std::round(sample / step) * step;
                sample *= (1.0 + driveAmount * 1.2);
            }
            break;
    }
}

void AcidVoice::processFilter(double& sample, double cutoffLFOValue, double resonanceLFOValue, float modulatedEnvMod, float filterEnvValue)
{
    // State-variable filter (resonant low-pass)
    // This gives that classic TB-303 sound

    // Calculate filter frequency with envelope modulation
    double modulation = filterEnvValue * modulatedEnvMod * 8000.0;

    // Add dedicated cutoff LFO modulation
    modulation += cutoffLFOValue * cutoffLFO.depth * 3000.0; // LFO can modulate +/- 3kHz

    double modulatedCutoff = juce::jlimit(20.0, 20000.0, filterCutoff + modulation);

    // Calculate filter coefficients
    double f = 2.0 * std::sin(juce::MathConstants<double>::pi * modulatedCutoff / sampleRate);
    f = juce::jlimit(0.0, 1.0, f);

    // State-variable filter implementation
    // Apply dedicated resonance LFO modulation
    // Note: Negate LFO value because resonance gets inverted later (1.0 - modulatedResonance)
    double modulatedResonance = filterResonance - resonanceLFOValue * resonanceLFO.depth * 0.5;
    modulatedResonance = juce::jlimit(0.0, 1.5, modulatedResonance); // Allow up to 1.5 for self-oscillation

    // Apply filter feedback for analog-style resonance
    // Feedback adds saturation at the resonant peak, creating that "smack"
    double feedbackAmount = filterFeedback * modulatedResonance;
    if (feedbackAmount > 0.01)
    {
        // Saturate the feedback signal for analog character
        double feedbackSample = filter2 * feedbackAmount;
        feedbackSample = std::tanh(feedbackSample * 2.0); // Saturate feedback path
        sample += feedbackSample;
    }

    // Invert resonance: higher filterResonance = less damping = more resonance
    // For very high resonance (> 1.0), use negative damping for self-oscillation
    double damping;
    if (modulatedResonance > 1.0)
    {
        // Self-oscillation range - negative damping
        damping = juce::jlimit(-0.2, 0.0, 1.0 - modulatedResonance);
    }
    else
    {
        // Normal range
        damping = juce::jlimit(0.01, 1.0, 1.0 - modulatedResonance);
    }

    double lowpass = filter2 + f * filter1;
    double highpass = sample - lowpass - damping * filter1;
    double bandpass = f * highpass + filter1;

    // Clamp filter states to prevent runaway values
    // Higher limits for self-oscillation
    filter1 = juce::jlimit(-15.0, 15.0, bandpass);
    filter2 = juce::jlimit(-15.0, 15.0, lowpass);

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
