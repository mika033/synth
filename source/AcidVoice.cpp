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

    // Setup ADSR for noise decay envelope (default values)
    noiseADSRParams.attack = 0.001f;  // Very fast attack
    noiseADSRParams.decay = 0.0f;     // No decay (will be set by noiseDecay parameter)
    noiseADSRParams.sustain = 0.0f;   // No sustain
    noiseADSRParams.release = 0.01f;  // Very short release
    noiseADSR.setParameters(noiseADSRParams);

    // Initialize drift phases with different starting positions for independence
    driftPhase1 = 0.0;
    driftPhase2 = juce::MathConstants<double>::twoPi / 3.0;  // 120° offset
    driftPhase3 = 2.0 * juce::MathConstants<double>::twoPi / 3.0;  // 240° offset
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

    // Phase randomization: randomize or reset oscillator starting phases
    if (phaseRandomAmount > 0.01f)
    {
        juce::Random random;
        osc1.angle = random.nextFloat() * juce::MathConstants<double>::twoPi * phaseRandomAmount;
        osc2.angle = random.nextFloat() * juce::MathConstants<double>::twoPi * phaseRandomAmount;
        osc3.angle = random.nextFloat() * juce::MathConstants<double>::twoPi * phaseRandomAmount;
    }
    else
    {
        // Reset to fully aligned state when phase random is at 0
        osc1.angle = 0.0;
        osc2.angle = 0.0;
        osc3.angle = 0.0;
    }

    // Initialize unison voice angles with phase offsets
    for (int v = 0; v < maxUnisonVoices; ++v)
    {
        unisonAngles1[v] = osc1.angle + unisonPhaseOffsets[v];
        unisonAngles2[v] = osc2.angle + unisonPhaseOffsets[v];
        unisonAngles3[v] = osc3.angle + unisonPhaseOffsets[v];
    }

    // Update frequency
    updateAngleDelta();

    // Start all ADSRs
    ampADSR.noteOn();
    filterADSR.noteOn();
    noiseADSR.noteOn();
}

void AcidVoice::stopNote(float /*velocity*/, bool allowTailOff)
{
    ampADSR.noteOff();
    filterADSR.noteOff();
    noiseADSR.noteOff();

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

        // Drift: Apply slow random pitch modulation (per-oscillator)
        if (driftAmount > 0.01f)
        {
            // Oscillator 1 drift
            driftPhase1 += driftFrequency * juce::MathConstants<double>::twoPi / sampleRate;
            if (driftPhase1 > juce::MathConstants<double>::twoPi)
                driftPhase1 -= juce::MathConstants<double>::twoPi;
            double driftSine1 = std::sin(driftPhase1);
            double driftCents1 = driftSine1 * driftAmount * 3.0; // ±3 cents max
            driftPitchRatio1 = std::pow(2.0, driftCents1 / 1200.0);

            // Oscillator 2 drift (phase offset for independence)
            driftPhase2 += driftFrequency * juce::MathConstants<double>::twoPi / sampleRate;
            if (driftPhase2 > juce::MathConstants<double>::twoPi)
                driftPhase2 -= juce::MathConstants<double>::twoPi;
            double driftSine2 = std::sin(driftPhase2);
            double driftCents2 = driftSine2 * driftAmount * 3.0;
            driftPitchRatio2 = std::pow(2.0, driftCents2 / 1200.0);

            // Oscillator 3 drift (different phase offset)
            driftPhase3 += driftFrequency * juce::MathConstants<double>::twoPi / sampleRate;
            if (driftPhase3 > juce::MathConstants<double>::twoPi)
                driftPhase3 -= juce::MathConstants<double>::twoPi;
            double driftSine3 = std::sin(driftPhase3);
            double driftCents3 = driftSine3 * driftAmount * 3.0;
            driftPitchRatio3 = std::pow(2.0, driftCents3 / 1200.0);
        }
        else
        {
            driftPitchRatio1 = 1.0; // No drift
            driftPitchRatio2 = 1.0;
            driftPitchRatio3 = 1.0;
        }

        // Apply waveform LFO modulation to Oscillator 1
        float modulatedWaveform = osc1.wave + static_cast<float>(waveformLFOValue) * waveformLFO.depth;
        modulatedWaveform = juce::jlimit(0.0f, 1.0f, modulatedWaveform);

        // Temporarily set the waveform for this sample
        float originalWaveform = osc1.wave;
        osc1.wave = modulatedWaveform;

        // Apply sub-oscillator LFO modulation to Oscillator 3 mix
        float modulatedSubOsc = osc3.mix + static_cast<float>(subOscLFOValue) * subOscLFO.depth;
        modulatedSubOsc = juce::jlimit(0.0f, 1.0f, modulatedSubOsc);

        float originalSubOscMix = osc3.mix;
        osc3.mix = modulatedSubOsc;

        // Get noise envelope value
        float noiseEnvValue = noiseADSR.getNextSample();

        // Generate mixed oscillator sample (all three oscillators + noise with envelope)
        double sample = generateOscillator();

        // Apply noise envelope to the noise portion
        // We need to separate this, so let's generate noise separately
        double noiseSample = 0.0;
        if (noiseMix > 0.01f)
        {
            // Generate white noise (-1 to +1)
            double whiteNoise = noiseRandom.nextFloat() * 2.0f - 1.0f;

            // Generate pink noise using simple one-pole filter
            pinkNoiseB0 = 0.99765 * pinkNoiseB0 + whiteNoise * 0.0990460;
            pinkNoiseB1 = 0.96300 * pinkNoiseB1 + whiteNoise * 0.2965164;
            pinkNoiseB2 = 0.57000 * pinkNoiseB2 + whiteNoise * 1.0526913;
            double pinkNoise = pinkNoiseB0 + pinkNoiseB1 + pinkNoiseB2 + whiteNoise * 0.1848;
            pinkNoise *= 0.11; // Normalize

            // Generate filtered noise (bandpass ~500Hz-2kHz for percussive sound)
            static double filteredB1 = 0.0, filteredB2 = 0.0;
            double f = 0.15; // Filter frequency coefficient
            double q = 0.5;  // Resonance
            double lowpass = filteredB2 + f * filteredB1;
            double highpass = whiteNoise - lowpass - q * filteredB1;
            double bandpass = f * highpass + filteredB1;
            filteredB1 = bandpass;
            filteredB2 = lowpass;
            double filteredNoise = bandpass * 3.0; // Amplify bandpass output

            // Morph between noise types
            double morphedNoise;
            if (noiseType < 0.5f)
            {
                float blend = noiseType * 2.0f;
                morphedNoise = whiteNoise * (1.0 - blend) + pinkNoise * blend;
            }
            else
            {
                float blend = (noiseType - 0.5f) * 2.0f;
                morphedNoise = pinkNoise * (1.0 - blend) + filteredNoise * blend;
            }

            // Apply noise envelope and mix
            noiseSample = morphedNoise * noiseMix * noiseEnvValue;
        }

        // Add noise to the sample
        sample += noiseSample;

        // Restore original values
        osc1.wave = originalWaveform;
        osc3.mix = originalSubOscMix;

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
        // Apply per-oscillator drift modulation to angle increment
        osc1.angle += osc1.angleDelta * driftPitchRatio1;
        osc1.angleDelta = osc1.targetAngleDelta; // Instant pitch change (no slide)
        if (osc1.angle > juce::MathConstants<double>::twoPi)
            osc1.angle -= juce::MathConstants<double>::twoPi;

        osc2.angle += osc2.angleDelta * driftPitchRatio2;
        osc2.angleDelta = osc2.targetAngleDelta;
        if (osc2.angle > juce::MathConstants<double>::twoPi)
            osc2.angle -= juce::MathConstants<double>::twoPi;

        osc3.angle += osc3.angleDelta * driftPitchRatio3;
        osc3.angleDelta = osc3.targetAngleDelta;
        if (osc3.angle > juce::MathConstants<double>::twoPi)
            osc3.angle -= juce::MathConstants<double>::twoPi;

        // Advance unison voice angles with frequency detuning
        if (unisonAmount > 0.01f)
        {
            for (int v = 0; v < maxUnisonVoices; ++v)
            {
                // Calculate detune ratio for this voice (±10 cents max)
                float detuneCents = unisonDetuneAmounts[v] * unisonAmount * 10.0f;
                double detunePitchRatio = std::pow(2.0, detuneCents / 1200.0);

                // Advance each unison voice with its own detuned frequency
                unisonAngles1[v] += osc1.angleDelta * driftPitchRatio1 * detunePitchRatio;
                if (unisonAngles1[v] > juce::MathConstants<double>::twoPi)
                    unisonAngles1[v] -= juce::MathConstants<double>::twoPi;

                unisonAngles2[v] += osc2.angleDelta * driftPitchRatio2 * detunePitchRatio;
                if (unisonAngles2[v] > juce::MathConstants<double>::twoPi)
                    unisonAngles2[v] -= juce::MathConstants<double>::twoPi;

                unisonAngles3[v] += osc3.angleDelta * driftPitchRatio3 * detunePitchRatio;
                if (unisonAngles3[v] > juce::MathConstants<double>::twoPi)
                    unisonAngles3[v] -= juce::MathConstants<double>::twoPi;
            }
        }

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
        noiseADSR.setSampleRate(newRate);
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

void AcidVoice::setOscillator1(float wave, int coarse, float fine, float mix)
{
    osc1.wave = juce::jlimit(0.0f, 1.0f, wave);
    osc1.coarseTune = juce::jlimit(-24, 24, coarse);
    osc1.fineTune = juce::jlimit(-100.0f, 100.0f, fine);
    osc1.mix = juce::jlimit(0.0f, 1.0f, mix);
    updateAngleDelta(); // Recalculate frequencies
}

void AcidVoice::setOscillator2(float wave, int coarse, float fine, float mix)
{
    osc2.wave = juce::jlimit(0.0f, 1.0f, wave);
    osc2.coarseTune = juce::jlimit(-24, 24, coarse);
    osc2.fineTune = juce::jlimit(-100.0f, 100.0f, fine);
    osc2.mix = juce::jlimit(0.0f, 1.0f, mix);
    updateAngleDelta(); // Recalculate frequencies
}

void AcidVoice::setOscillator3(float wave, int coarse, float fine, float mix)
{
    osc3.wave = juce::jlimit(0.0f, 1.0f, wave);
    osc3.coarseTune = juce::jlimit(-24, 24, coarse);
    osc3.fineTune = juce::jlimit(-100.0f, 100.0f, fine);
    osc3.mix = juce::jlimit(0.0f, 1.0f, mix);
    updateAngleDelta(); // Recalculate frequencies
}

void AcidVoice::setNoiseMix(float mix)
{
    noiseMix = juce::jlimit(0.0f, 1.0f, mix);
}

void AcidVoice::setNoiseType(float type)
{
    noiseType = juce::jlimit(0.0f, 1.0f, type);
}

void AcidVoice::setNoiseDecay(float decay)
{
    noiseDecay = juce::jlimit(0.0f, 2.0f, decay);
    // Update noise ADSR decay time
    // Right (2.0) = sustained, Left (0.0) = shortest decay (0.01s)
    if (noiseDecay > 1.99f)
    {
        // No decay - sustained noise (dial to the right)
        noiseADSRParams.decay = 0.0f;
        noiseADSRParams.sustain = 1.0f; // Full sustain
    }
    else
    {
        // Percussive decay envelope with minimum 0.01s
        noiseADSRParams.decay = std::max(0.01f, noiseDecay);
        noiseADSRParams.sustain = 0.0f; // No sustain
    }
    noiseADSR.setParameters(noiseADSRParams);
}

void AcidVoice::setDrive(float drive)
{
    driveAmount = juce::jlimit(0.0f, 1.0f, drive);
}

void AcidVoice::setVolume(float volume)
{
    volumeLevel = juce::jlimit(0.0f, 1.0f, volume);
}

void AcidVoice::setGlobalOctave(int octave)
{
    globalOctaveShift = juce::jlimit(-2, 2, octave);
    updateAngleDelta(); // Recalculate frequency with new octave shift
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

// Analog character setters
void AcidVoice::setDrift(float amount)
{
    driftAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void AcidVoice::setPhaseRandom(float amount)
{
    phaseRandomAmount = juce::jlimit(0.0f, 1.0f, amount);
}

void AcidVoice::setUnison(float amount)
{
    unisonAmount = juce::jlimit(0.0f, 1.0f, amount);
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

double AcidVoice::generateSingleOscillator(Oscillator& osc)
{
    // Generate three waveforms: sine, sawtooth, square
    double sineSample = std::sin(osc.angle);
    double sawtoothSample = (osc.angle / juce::MathConstants<double>::pi) - 1.0;
    double squareSample = osc.angle < juce::MathConstants<double>::pi ? 1.0 : -1.0;

    // Morph between waveforms based on wave parameter (0=sine, 0.5=saw, 1=square)
    double sample;
    if (osc.wave < 0.5f)
    {
        // Morph from sine (0) to sawtooth (0.5)
        float blend = osc.wave * 2.0f; // Map 0-0.5 to 0-1
        sample = sineSample * (1.0 - blend) + sawtoothSample * blend;
    }
    else
    {
        // Morph from sawtooth (0.5) to square (1.0)
        float blend = (osc.wave - 0.5f) * 2.0f; // Map 0.5-1.0 to 0-1
        sample = sawtoothSample * (1.0 - blend) + squareSample * blend;
    }

    return sample;
}

double AcidVoice::generateOscillator()
{
    double osc1Sample = 0.0;
    double osc2Sample = 0.0;
    double osc3Sample = 0.0;

    // Unison: Always render 3 voices, dial controls detune amount only
    if (unisonAmount > 0.01f)
    {
        // Always use 3 voices with level compensation
        float levelCompensation = 1.0f / std::sqrt(3.0f);

        // Oscillator 1 unison (3 voices)
        for (int v = 0; v < 3; ++v)
        {
            double originalAngle = osc1.angle;
            osc1.angle = unisonAngles1[v];

            double sample = generateSingleOscillator(osc1);
            osc1Sample += sample * levelCompensation;

            osc1.angle = originalAngle;
        }
        osc1Sample *= osc1.mix;

        // Oscillator 2 unison (3 voices)
        for (int v = 0; v < 3; ++v)
        {
            double originalAngle = osc2.angle;
            osc2.angle = unisonAngles2[v];

            double sample = generateSingleOscillator(osc2);
            osc2Sample += sample * levelCompensation;

            osc2.angle = originalAngle;
        }
        osc2Sample *= osc2.mix;

        // Oscillator 3 unison (3 voices)
        for (int v = 0; v < 3; ++v)
        {
            double originalAngle = osc3.angle;
            osc3.angle = unisonAngles3[v];

            double sample = generateSingleOscillator(osc3);
            osc3Sample += sample * levelCompensation;

            osc3.angle = originalAngle;
        }
        osc3Sample *= osc3.mix;
    }
    else
    {
        // Normal mode: single voice per oscillator
        osc1Sample = generateSingleOscillator(osc1) * osc1.mix;
        osc2Sample = generateSingleOscillator(osc2) * osc2.mix;
        osc3Sample = generateSingleOscillator(osc3) * osc3.mix;
    }

    // Mix the oscillators (noise is now generated in renderNextBlock with envelope)
    return osc1Sample + osc2Sample + osc3Sample;
}

void AcidVoice::applySaturation(double& sample)
{
    if (driveAmount < 0.001f)
        return;

    // Pre-gain based on drive amount
    double gain = 1.0 + driveAmount * 9.0; // Up to 10x gain
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
    // Base frequency from MIDI note with global octave shift
    double baseFreq = juce::MidiMessage::getMidiNoteInHertz(currentMidiNote);
    baseFreq *= std::pow(2.0, globalOctaveShift);

    // Calculate frequency for each oscillator with coarse and fine tuning
    // Coarse: semitones (12 semitones = 1 octave)
    // Fine: cents (100 cents = 1 semitone)

    // Oscillator 1
    double osc1Freq = baseFreq * std::pow(2.0, osc1.coarseTune / 12.0) * std::pow(2.0, osc1.fineTune / 1200.0);
    osc1.targetAngleDelta = osc1Freq * juce::MathConstants<double>::twoPi / sampleRate;

    // Oscillator 2
    double osc2Freq = baseFreq * std::pow(2.0, osc2.coarseTune / 12.0) * std::pow(2.0, osc2.fineTune / 1200.0);
    osc2.targetAngleDelta = osc2Freq * juce::MathConstants<double>::twoPi / sampleRate;

    // Oscillator 3
    double osc3Freq = baseFreq * std::pow(2.0, osc3.coarseTune / 12.0) * std::pow(2.0, osc3.fineTune / 1200.0);
    osc3.targetAngleDelta = osc3Freq * juce::MathConstants<double>::twoPi / sampleRate;
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
