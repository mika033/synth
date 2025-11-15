#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * UI Editor for Acid Synth plugin
 */
class AcidSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AcidSynthAudioProcessorEditor(AcidSynthAudioProcessor&);
    ~AcidSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& audioProcessor;

    // Sliders
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider envModSlider;
    juce::Slider decaySlider;
    juce::Slider accentSlider;
    juce::Slider subOscSlider;
    juce::Slider driveSlider;
    juce::Slider volumeSlider;
    juce::Slider lfoDepthSlider;
    juce::Slider delayFeedbackSlider;
    juce::Slider delayMixSlider;
    juce::ComboBox waveformSelector;
    juce::ComboBox presetSelector;
    juce::ComboBox lfoRateSelector;
    juce::ComboBox lfoDestSelector;
    juce::ComboBox delayTimeSelector;

    // Labels
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label envModLabel;
    juce::Label decayLabel;
    juce::Label accentLabel;
    juce::Label subOscLabel;
    juce::Label driveLabel;
    juce::Label volumeLabel;
    juce::Label waveformLabel;
    juce::Label presetLabel;
    juce::Label lfoRateLabel;
    juce::Label lfoDestLabel;
    juce::Label lfoDepthLabel;
    juce::Label delayTimeLabel;
    juce::Label delayFeedbackLabel;
    juce::Label delayMixLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envModAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subOscAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lfoDestAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> delayTimeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcidSynthAudioProcessorEditor)
};
