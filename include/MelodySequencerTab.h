#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Custom LookAndFeel for vertical fill bar sliders
 */
class VerticalFillBarLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        // Draw background (dark gray rounded rectangle, like step buttons)
        g.setColour(juce::Colour(0xff404040));
        g.fillRoundedRectangle(x, y, width, height, 5.0f);

        // Calculate fill height based on slider value (-1.0 to 1.0 range)
        float value = (float)slider.getValue();
        float normalizedValue = (value + 1.0f) / 2.0f; // Convert -1..1 to 0..1
        float fillHeight = height * normalizedValue;

        // Draw green fill from bottom
        if (fillHeight > 0)
        {
            g.setColour(juce::Colour(0xff00aa00)); // Same green as step buttons
            float fillY = y + (height - fillHeight);
            g.fillRoundedRectangle((float)x, fillY, (float)width, fillHeight, 5.0f);
        }

        // Draw border (white like step buttons)
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawRoundedRectangle(x, y, width, height, 5.0f, 1.0f);
    }
};

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

    // Steps and Rate controls
    juce::Slider stepsSlider;
    juce::Label stepsLabel;
    juce::ComboBox rateSelector;
    juce::Label rateLabel;

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

    // Custom LookAndFeel for cutoff fill bars
    VerticalFillBarLookAndFeel fillBarLookAndFeel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAttachment;

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
