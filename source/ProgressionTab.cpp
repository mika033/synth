#include "PluginProcessor.h"
#include "ProgressionTab.h"

//==============================================================================
ProgressionTab::ProgressionTab(SnorkelSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Configure enable toggle
    enableToggle.setButtonText("Enabled");
    addAndMakeVisible(enableToggle);
    enableLabel.setText("Progression", juce::dontSendNotification);
    enableLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(enableLabel);

    // Attach enable toggle to parameter
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "progenabled", enableToggle);

    // Configure steps slider
    stepsSlider.setSliderStyle(juce::Slider::IncDecButtons);
    stepsSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    stepsSlider.setRange(1, 16, 1);
    stepsSlider.setValue(4);
    stepsSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    addAndMakeVisible(stepsSlider);
    stepsLabel.setText("Steps", juce::dontSendNotification);
    stepsLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(stepsLabel);
    stepsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "progsteps", stepsSlider);

    // Listen to steps changes to update visibility
    stepsSlider.onValueChange = [this]() { resized(); };

    // Configure length selector
    lengthSelector.addItem("1/2", 1);
    lengthSelector.addItem("1", 2);
    lengthSelector.addItem("2", 3);
    lengthSelector.addItem("3", 4);
    lengthSelector.addItem("4", 5);
    lengthSelector.setSelectedId(2); // Default to 1 bar
    addAndMakeVisible(lengthSelector);
    lengthLabel.setText("Length (bars)", juce::dontSendNotification);
    lengthLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(lengthLabel);
    lengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "proglength", lengthSelector);

    // Configure Random button
    randomButton.setButtonText("Random");
    randomButton.onClick = [this]() { randomizeProgression(); };
    addAndMakeVisible(randomButton);

    // Configure 16 step sliders
    const char* paramIds[] = {
        "progstep1", "progstep2", "progstep3", "progstep4",
        "progstep5", "progstep6", "progstep7", "progstep8",
        "progstep9", "progstep10", "progstep11", "progstep12",
        "progstep13", "progstep14", "progstep15", "progstep16"
    };

    for (int i = 0; i < NUM_STEPS; ++i)
    {
        // Configure slider as text box with drag to change
        stepSliders[i].setSliderStyle(juce::Slider::IncDecButtons);
        stepSliders[i].setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 40);
        stepSliders[i].setRange(1, 8, 1);
        stepSliders[i].setValue(1);
        stepSliders[i].setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);

        // Style the box to look similar to sequencer boxes
        stepSliders[i].setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        stepSliders[i].setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff404040));
        stepSliders[i].setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff606060));

        addAndMakeVisible(stepSliders[i]);

        // Configure label
        stepLabels[i].setText("Step " + juce::String(i + 1), juce::dontSendNotification);
        stepLabels[i].setJustificationType(juce::Justification::centred);
        stepLabels[i].setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible(stepLabels[i]);

        // Attach to parameter
        stepAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), paramIds[i], stepSliders[i]);
    }

    // Start timer to update current step visualization
    startTimer(50); // Update at 20Hz
}

ProgressionTab::~ProgressionTab()
{
    stopTimer();
}

//==============================================================================
void ProgressionTab::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw panel for progression controls - make it taller to accommodate two rows
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(30, 70, getWidth() - 60, 360, 10);

    // Highlight current step
    if (currentStep >= 0 && currentStep < NUM_STEPS)
    {
        const int startX = 80;
        const int horizontalSpacing = 90;
        const int row1Y = 120;
        const int row2Y = row1Y + 90;
        const int boxWidth = 60;
        const int boxHeight = 60;
        const int labelHeight = 20;

        // Only highlight if this step is active (within the configured number of steps)
        int numSteps = static_cast<int>(audioProcessor.getValueTreeState().getRawParameterValue("progsteps")->load());
        if (currentStep < numSteps)
        {
            // Calculate position based on row (0-7 first row, 8-15 second row)
            int row = currentStep / 8;
            int col = currentStep % 8;
            int yPos = (row == 0) ? row1Y : row2Y;

            g.setColour(juce::Colours::orange.withAlpha(0.3f));
            int x = startX + col * horizontalSpacing;
            g.fillRect(x - 5, yPos - labelHeight - 10, boxWidth + 10, boxHeight + labelHeight + 15);
        }
    }
}

void ProgressionTab::resized()
{
    const int startX = 80;

    // Enable toggle at the top
    enableLabel.setBounds(20, 20, 120, 25);
    enableToggle.setBounds(140, 20, 80, 30);

    // Steps control
    stepsLabel.setBounds(240, 20, 60, 25);
    stepsSlider.setBounds(310, 20, 60, 30);

    // Length control
    lengthLabel.setBounds(390, 20, 100, 25);
    lengthSelector.setBounds(500, 20, 80, 25);

    // Random button
    randomButton.setBounds(600, 20, 80, 25);

    // Get current number of active steps
    int numActiveSteps = static_cast<int>(stepsSlider.getValue());

    // Layout parameters
    const int boxWidth = 60;
    const int boxHeight = 60;
    const int horizontalSpacing = 90;
    const int row1Y = 120;
    const int row2Y = row1Y + 90; // Second row below first
    const int labelHeight = 20;

    // Position steps 1-8 in first row, steps 9-16 in second row
    for (int i = 0; i < NUM_STEPS; ++i)
    {
        bool isVisible = (i < numActiveSteps);
        stepLabels[i].setVisible(isVisible);
        stepSliders[i].setVisible(isVisible);

        if (isVisible)
        {
            int row = i / 8; // 0 for steps 1-8, 1 for steps 9-16
            int col = i % 8; // Column within the row
            int yPos = (row == 0) ? row1Y : row2Y;

            stepLabels[i].setBounds(startX + col * horizontalSpacing, yPos - labelHeight - 5, boxWidth, labelHeight);
            stepSliders[i].setBounds(startX + col * horizontalSpacing, yPos, boxWidth, boxHeight);
        }
    }
}

void ProgressionTab::timerCallback()
{
    // Update current step from processor
    int newStep = audioProcessor.currentProgressionStep;
    if (newStep != currentStep)
    {
        currentStep = newStep;
        repaint();
    }
}

void ProgressionTab::randomizeProgression()
{
    // Get number of active steps
    int numSteps = static_cast<int>(stepsSlider.getValue());

    if (numSteps < 1)
        return;

    // Weights for scale degrees (1-8) based on musicality
    // Index: 0=degree1, 1=degree2, 2=degree3, 3=degree4, 4=degree5, 5=degree6, 6=degree7, 7=degree8
    int degreeWeights[] = {25, 10, 5, 20, 20, 15, 3, 2}; // Total: 100
    int totalWeight = 100;

    // Step 1: 75% chance of being root (degree 1)
    bool step1IsRoot = (random.nextFloat() < 0.75f);

    if (step1IsRoot)
    {
        stepSliders[0].setValue(1);
    }
    else
    {
        // Choose a non-root degree for step 1
        int roll = random.nextInt(totalWeight - degreeWeights[0]); // Exclude degree 1
        int cumulative = 0;
        int selectedDegree = 1; // default

        for (int i = 1; i < 8; ++i) // Start from degree 2
        {
            cumulative += degreeWeights[i];
            if (roll < cumulative)
            {
                selectedDegree = i + 1; // Convert to 1-8 range
                break;
            }
        }
        stepSliders[0].setValue(selectedDegree);
    }

    // Randomize middle steps (2 to numSteps-1) using weighted random
    for (int step = 1; step < numSteps - 1; ++step)
    {
        int roll = random.nextInt(totalWeight);
        int cumulative = 0;
        int selectedDegree = 1; // default

        for (int i = 0; i < 8; ++i)
        {
            cumulative += degreeWeights[i];
            if (roll < cumulative)
            {
                selectedDegree = i + 1; // Convert to 1-8 range
                break;
            }
        }
        stepSliders[step].setValue(selectedDegree);
    }

    // Last step: if step 1 was not root, force it to be root
    if (numSteps > 1)
    {
        if (!step1IsRoot)
        {
            stepSliders[numSteps - 1].setValue(1);
        }
        else
        {
            // Use weighted random for last step
            int roll = random.nextInt(totalWeight);
            int cumulative = 0;
            int selectedDegree = 1; // default

            for (int i = 0; i < 8; ++i)
            {
                cumulative += degreeWeights[i];
                if (roll < cumulative)
                {
                    selectedDegree = i + 1; // Convert to 1-8 range
                    break;
                }
            }
            stepSliders[numSteps - 1].setValue(selectedDegree);
        }
    }
}
