#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Synth Tab - Contains all synthesizer controls with dedicated LFOs
 */
class SynthTab : public juce::Component
{
public:
    SynthTab(AcidSynthAudioProcessor& p);
    ~SynthTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& audioProcessor;

    // Helper structure for LFO controls (each parameter gets one)
    struct LFOControls
    {
        juce::ComboBox rateSelector;
        juce::ComboBox waveformSelector;
        juce::Slider depthSlider;
        juce::Label rateLabel;
        juce::Label waveformLabel;
        juce::Label depthLabel;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    };

    // Main parameter sliders
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider envModSlider;
    juce::Slider decaySlider;
    juce::Slider accentSlider;
    juce::Slider waveformSlider;
    juce::Slider subOscSlider;
    juce::Slider driveSlider;
    juce::Slider volumeSlider;
    juce::Slider delayFeedbackSlider;
    juce::Slider delayMixSlider;
    juce::ComboBox delayTimeSelector;
    juce::ComboBox presetSelector;

    // Main parameter labels
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label envModLabel;
    juce::Label decayLabel;
    juce::Label accentLabel;
    juce::Label waveformLabel;
    juce::Label subOscLabel;
    juce::Label driveLabel;
    juce::Label volumeLabel;
    juce::Label delayTimeLabel;
    juce::Label delayFeedbackLabel;
    juce::Label delayMixLabel;
    juce::Label presetLabel;

    // 10 Dedicated LFO control sets
    LFOControls cutoffLFO;
    LFOControls resonanceLFO;
    LFOControls envModLFO;
    LFOControls decayLFO;
    LFOControls accentLFO;
    LFOControls waveformLFO;
    LFOControls subOscLFO;
    LFOControls driveLFO;
    LFOControls volumeLFO;
    LFOControls delayMixLFO;

    // Main parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envModAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> waveformAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subOscAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> delayTimeAttachment;

    // Helper function to setup LFO controls
    void setupLFOControls(LFOControls& lfo, const juce::String& rateParamID,
                          const juce::String& waveParamID, const juce::String& depthParamID);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthTab)
};
