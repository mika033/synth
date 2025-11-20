#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Melody Sequencer Tab - Contains 16-step melodic sequencer
 */
class MelodySequencerTab : public juce::Component, private juce::Timer
{
public:
    MelodySequencerTab(SnorkelSynthAudioProcessor& p);
    ~MelodySequencerTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SnorkelSynthAudioProcessor& audioProcessor;

    // Enable toggle
    juce::ToggleButton enableToggle;
    juce::Label enableLabel;

    // Preset selector and save button
    juce::ComboBox presetSelector;
    juce::Label presetLabel;
    juce::TextButton savePresetButton;

    // Random and Mutate buttons
    juce::TextButton randomButton;
    juce::TextButton mutateButton;

    // Algorithm selector for randomization
    juce::ComboBox algoSelector;
    juce::Label algoLabel;

    // Scale and root selection
    juce::ComboBox scaleSelector;
    juce::ComboBox rootNoteSelector;
    juce::Label scaleLabel;
    juce::Label rootNoteLabel;

    // 16 steps x 8 buttons (scale degrees)
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_SCALE_DEGREES = 8;
    juce::TextButton stepButtons[NUM_STEPS][NUM_SCALE_DEGREES];

    // Per-step octave controls (16 steps) - two buttons per step
    juce::TextButton octaveUpButtons[NUM_STEPS];
    juce::TextButton octaveDownButtons[NUM_STEPS];

    // Per-step cutoff modulation dials (16 steps)
    juce::Slider cutoffSliders[NUM_STEPS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachments[NUM_STEPS];

    // Current step indicator
    int currentStep = 0;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rootNoteAttachment;

    // Button click handlers
    void onStepButtonClicked(int step, int degree);
    void updateButtonStates();
    void updateOctaveDisplay();
    void updateOctaveDisplayForStep(int step);
    void onOctaveUpClicked(int step);
    void onOctaveDownClicked(int step);
    void loadPreset(int presetIndex);
    void onRandomClicked();
    void generateTrueRandom();
    void generateWeightedRandom(int specificConfigIndex = -1); // -1 = random, 0-3 = specific config
    void onMutateClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelodySequencerTab)
};
