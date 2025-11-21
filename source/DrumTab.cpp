#include "DrumTab.h"

DrumTab::DrumTab(SnorkelSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Enable toggle
    enableToggle.setButtonText("");
    enableLabel.setText("Enable", juce::dontSendNotification);
    enableLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(enableToggle);
    addAndMakeVisible(enableLabel);
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "drumenable", enableToggle);

    // Create step button grid
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            auto& button = stepButtons[lane][step];
            button.setButtonText("");
            button.setClickingTogglesState(true);

            // Style buttons
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00aa00));

            button.onClick = [this, lane, step]()
            {
                onStepButtonClicked(lane, step);
            };

            addAndMakeVisible(button);
        }
    }

    // Per-lane volume sliders
    const char* laneVolumeIds[NUM_LANES] = { "drumkickvol", "drumsnarevol", "drumchatvol", "drumohatvol" };
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        laneVolumeSliders[lane].setSliderStyle(juce::Slider::RotaryVerticalDrag);
        laneVolumeSliders[lane].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        laneVolumeSliders[lane].setRange(0.0, 1.0, 0.01);
        laneVolumeSliders[lane].setValue(0.8);
        addAndMakeVisible(laneVolumeSliders[lane]);

        laneVolumeLabels[lane].setText("Vol", juce::dontSendNotification);
        laneVolumeLabels[lane].setJustificationType(juce::Justification::centred);
        laneVolumeLabels[lane].setFont(juce::Font(10.0f));
        addAndMakeVisible(laneVolumeLabels[lane]);

        laneVolumeAttachments[lane] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), laneVolumeIds[lane], laneVolumeSliders[lane]);
    }

    // Global volume slider
    globalVolumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    globalVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    globalVolumeSlider.setRange(0.0, 1.0, 0.01);
    globalVolumeSlider.setValue(0.8);
    addAndMakeVisible(globalVolumeSlider);

    globalVolumeLabel.setText("Master", juce::dontSendNotification);
    globalVolumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(globalVolumeLabel);

    globalVolumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drummastervol", globalVolumeSlider);

    // Sidechain Magnitude slider
    sidechainMagnitudeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    sidechainMagnitudeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sidechainMagnitudeSlider.setRange(0.0, 1.0, 0.01);
    sidechainMagnitudeSlider.setValue(0.0);
    addAndMakeVisible(sidechainMagnitudeSlider);

    sidechainMagnitudeLabel.setText("SC Mag", juce::dontSendNotification);
    sidechainMagnitudeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sidechainMagnitudeLabel);

    sidechainMagnitudeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drumsidechainmag", sidechainMagnitudeSlider);

    // Sidechain Length slider
    sidechainLengthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    sidechainLengthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sidechainLengthSlider.setRange(0.0, 1.0, 0.01);
    sidechainLengthSlider.setValue(0.5);
    addAndMakeVisible(sidechainLengthSlider);

    sidechainLengthLabel.setText("SC Len", juce::dontSendNotification);
    sidechainLengthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sidechainLengthLabel);

    sidechainLengthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drumsidechainlen", sidechainLengthSlider);

    // Load initial button states
    updateButtonStates();

    // Start timer to update current step visualization
    startTimer(50);
}

DrumTab::~DrumTab()
{
    stopTimer();
}

void DrumTab::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw lane labels
    g.setColour(juce::Colours::white);
    g.setFont(14.0f);

    const int startY = 80;
    const int buttonSize = 40;
    const int verticalSpacing = 50;

    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        int y = startY + lane * verticalSpacing;
        g.drawText(laneNames[lane], 10, y, 50, buttonSize, juce::Justification::centredRight);
    }

    // Highlight current step
    if (currentStep >= 0 && currentStep < NUM_STEPS)
    {
        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        int x = 70 + currentStep * 50;
        int height = NUM_LANES * verticalSpacing;
        g.fillRect(x, startY, buttonSize, height);
    }
}

void DrumTab::resized()
{
    auto bounds = getLocalBounds();

    // Top controls
    const int controlY = 10;
    const int controlHeight = 25;

    enableToggle.setBounds(20, controlY, 25, controlHeight);
    enableLabel.setBounds(50, controlY, 60, controlHeight);

    // Global volume
    globalVolumeSlider.setBounds(150, controlY - 5, 50, 50);
    globalVolumeLabel.setBounds(150, controlY + 40, 50, 20);

    // Sidechain controls
    sidechainMagnitudeSlider.setBounds(220, controlY - 5, 50, 50);
    sidechainMagnitudeLabel.setBounds(220, controlY + 40, 50, 20);

    sidechainLengthSlider.setBounds(280, controlY - 5, 50, 50);
    sidechainLengthLabel.setBounds(280, controlY + 40, 50, 20);

    // Step button grid
    const int startX = 70;
    const int startY = 80;
    const int buttonSize = 40;
    const int horizontalSpacing = 50;
    const int verticalSpacing = 50;

    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            stepButtons[lane][step].setBounds(
                startX + step * horizontalSpacing,
                startY + lane * verticalSpacing,
                buttonSize,
                buttonSize
            );
        }
    }

    // Per-lane volume sliders (to the right of grid)
    const int volumeX = startX + NUM_STEPS * horizontalSpacing + 20;
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        int y = startY + lane * verticalSpacing - 5;
        laneVolumeSliders[lane].setBounds(volumeX, y, 45, 45);
        laneVolumeLabels[lane].setBounds(volumeX, y + 40, 45, 15);
    }
}

void DrumTab::timerCallback()
{
    int newStep = audioProcessor.currentDrumStep;
    if (newStep != currentStep)
    {
        currentStep = newStep;
        repaint();
    }
}

void DrumTab::onStepButtonClicked(int lane, int step)
{
    bool isOn = stepButtons[lane][step].getToggleState();
    audioProcessor.drumPattern[lane][step] = isOn ? 1 : 0;
}

void DrumTab::updateButtonStates()
{
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            bool isOn = audioProcessor.drumPattern[lane][step] != 0;
            stepButtons[lane][step].setToggleState(isOn, juce::dontSendNotification);
        }
    }
}
