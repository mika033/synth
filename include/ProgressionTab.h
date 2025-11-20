#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Chord Progression tab for creating chord sequences
 */
class ProgressionTab : public juce::Component, private juce::Timer
{
public:
    ProgressionTab(SnorkelSynthAudioProcessor& p);
    ~ProgressionTab() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SnorkelSynthAudioProcessor& audioProcessor;

    // Randomization
    void randomizeProgression();
    juce::Random random;

    static constexpr int NUM_STEPS = 16;

    // Enable toggle
    juce::ToggleButton enableToggle;
    juce::Label enableLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;

    // Steps control
    juce::Slider stepsSlider;
    juce::Label stepsLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepsAttachment;

    // Length control
    juce::ComboBox lengthSelector;
    juce::Label lengthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> lengthAttachment;

    // Random button
    juce::TextButton randomButton;

    // 16 step sliders (displayed in two rows)
    juce::Slider stepSliders[NUM_STEPS];
    juce::Label stepLabels[NUM_STEPS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepAttachments[NUM_STEPS];

    // Current step tracking for visual feedback
    int currentStep = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProgressionTab)
};
