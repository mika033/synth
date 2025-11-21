#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Drum Machine Tab - 16-step drum sequencer with 4 lanes
 * Kick, Snare, Closed Hat, Open Hat
 */
class DrumTab : public juce::Component, private juce::Timer
{
public:
    DrumTab(SnorkelSynthAudioProcessor& p);
    ~DrumTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SnorkelSynthAudioProcessor& audioProcessor;

    // Constants
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_LANES = 4; // Kick, Snare, CHat, OHat

    // Lane names
    const juce::String laneNames[NUM_LANES] = { "Kick", "Snare", "CHat", "OHat" };

    // Enable toggle
    juce::ToggleButton enableToggle;
    juce::Label enableLabel;

    // Step buttons grid [lane][step]
    juce::TextButton stepButtons[NUM_LANES][NUM_STEPS];

    // Per-lane volume sliders
    juce::Slider laneVolumeSliders[NUM_LANES];
    juce::Label laneVolumeLabels[NUM_LANES];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> laneVolumeAttachments[NUM_LANES];

    // Global drum volume
    juce::Slider globalVolumeSlider;
    juce::Label globalVolumeLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> globalVolumeAttachment;

    // Sidechain controls (triggered by kick)
    juce::Slider sidechainMagnitudeSlider;
    juce::Label sidechainMagnitudeLabel;
    juce::Slider sidechainLengthSlider;
    juce::Label sidechainLengthLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sidechainMagnitudeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sidechainLengthAttachment;

    // Current step indicator
    int currentStep = 0;

    // Enable attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;

    // Button click handlers
    void onStepButtonClicked(int lane, int step);
    void updateButtonStates();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrumTab)
};
