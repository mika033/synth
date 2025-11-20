#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Filter Tab - Contains filter and envelope controls
 */
class FilterTab : public juce::Component
{
public:
    FilterTab(SnorkelSynthAudioProcessor& p);
    ~FilterTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SnorkelSynthAudioProcessor& audioProcessor;

    // Filter parameter sliders
    juce::Slider cutoffSlider;
    juce::Slider resonanceSlider;
    juce::Slider envModSlider;
    juce::Slider accentSlider;
    juce::Slider filterFeedbackSlider;
    juce::ComboBox saturationTypeSelector;

    // Filter ADSR sliders
    juce::Slider filterAttackSlider;
    juce::Slider filterDecaySlider;
    juce::Slider filterSustainSlider;
    juce::Slider filterReleaseSlider;

    // Delay controls
    juce::ComboBox delayTimeSelector;
    juce::Slider delayFeedbackSlider;
    juce::Slider delayMixSlider;

    // Preset controls
    juce::ComboBox presetSelector;
    juce::TextButton savePresetButton;
    juce::Label presetLabel;

    // Filter parameter labels
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label envModLabel;
    juce::Label accentLabel;
    juce::Label filterFeedbackLabel;
    juce::Label saturationTypeLabel;

    // Filter ADSR labels
    juce::Label filterAttackLabel;
    juce::Label filterDecayLabel;
    juce::Label filterSustainLabel;
    juce::Label filterReleaseLabel;

    // Delay labels
    juce::Label delayTimeLabel;
    juce::Label delayFeedbackLabel;
    juce::Label delayMixLabel;

    // Filter parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> resonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envModAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> saturationTypeAttachment;

    // Filter ADSR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterSustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterReleaseAttachment;

    // Delay attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> delayTimeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayFeedbackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterTab)
};
