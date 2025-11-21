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

// Custom button class for step grid that handles octave adjustment on active steps
class StepButton : public juce::TextButton
{
public:
    std::function<void(int, int, bool)> onStepClicked; // step, degree, wasAlreadyActive
    std::function<void(int, int, bool)> onOctaveAdjust; // step, degree, isIncrease
    int step = 0;
    int degree = 0;

    std::function<void(int)> onDeactivate; // step

    void mouseDown(const juce::MouseEvent& event) override
    {
        bool wasActive = getToggleState();

        if (wasActive)
        {
            // Button is already active - check which third was clicked
            int clickY = event.getMouseDownY();
            int buttonHeight = getHeight();
            int thirdHeight = buttonHeight / 3;

            if (clickY < thirdHeight)
            {
                // Upper third: octave +1
                if (onOctaveAdjust)
                    onOctaveAdjust(step, degree, true);
            }
            else if (clickY < thirdHeight * 2)
            {
                // Middle third: deactivate
                if (onDeactivate)
                    onDeactivate(step);
            }
            else
            {
                // Lower third: octave -1
                if (onOctaveAdjust)
                    onOctaveAdjust(step, degree, false);
            }

            // Don't change toggle state via normal click
            return;
        }
        else
        {
            // Button is not active - proceed with normal toggle
            juce::TextButton::mouseDown(event);
        }
    }

    void mouseUp(const juce::MouseEvent& event) override
    {
        // Only call mouseUp if button wasn't already active
        if (!getToggleState() || !contains(event.getPosition()))
        {
            juce::TextButton::mouseUp(event);
        }
    }
};

//==============================================================================
/**
 * Melody Sequencer Tab - Contains 16-step melodic sequencer
 */
class SnorkelSynthAudioProcessorEditor; // Forward declaration

class MelodySequencerTab : public juce::Component, private juce::Timer
{
public:
    MelodySequencerTab(SnorkelSynthAudioProcessor& p, SnorkelSynthAudioProcessorEditor& e);
    ~MelodySequencerTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    SnorkelSynthAudioProcessor& audioProcessor;
    SnorkelSynthAudioProcessorEditor& editor;

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

    // Steps, Rate, and Gate controls
    juce::Slider stepsSlider;
    juce::ComboBox rateSelector;
    juce::Slider gateSlider;

    // 16 steps x 8 buttons (scale degrees)
    static constexpr int NUM_STEPS = 16;
    static constexpr int NUM_SCALE_DEGREES = 8;
    StepButton stepButtons[NUM_STEPS][NUM_SCALE_DEGREES];

    // Per-step accent modulation (16 steps)
    juce::Slider accentSliders[NUM_STEPS];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentAttachments[NUM_STEPS];

    // Accent control dials (how accent row affects playback)
    juce::Slider accentVolSlider;
    juce::Slider accentCutoffSlider;
    juce::Slider accentResSlider;
    juce::Slider accentDecaySlider;
    juce::Slider accentDriveSlider;
    juce::Label accentVolLabel;
    juce::Label accentCutoffLabel;
    juce::Label accentResLabel;
    juce::Label accentDecayLabel;
    juce::Label accentDriveLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentVolAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentCutoffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentResAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> accentDriveAttachment;

    // Current step indicator
    int currentStep = 0;

    // Custom LookAndFeel for cutoff fill bars
    VerticalFillBarLookAndFeel fillBarLookAndFeel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> stepsAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateAttachment;

    // Button click handlers
    void onStepButtonClicked(int step, int degree);
    void updateButtonStates();
    void updateOctaveDisplay();
    void updateOctaveDisplayForStep(int step);
    void onOctaveUpClicked(int step, int degree);
    void onOctaveDownClicked(int step, int degree);
    void loadPreset(int presetIndex);
    void onRandomClicked();
    void generateTrueRandom();
    void generateWeightedRandom(int specificConfigIndex = -1); // -1 = random, 0-3 = specific config
    void onMutateClicked();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MelodySequencerTab)
};
