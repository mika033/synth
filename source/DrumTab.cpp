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

    // Pattern selection buttons (1-8)
    for (int i = 0; i < NUM_PATTERNS; ++i)
    {
        patternButtons[i].setButtonText(juce::String(i + 1));
        patternButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
        patternButtons[i].setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff0088ff));
        patternButtons[i].onClick = [this, i]() { onPatternButtonClicked(i); };
        addAndMakeVisible(patternButtons[i]);
    }
    updatePatternButtonStates();

    // Chain sequencer controls
    chainEnableToggle.setButtonText("");
    chainEnableLabel.setText("Chain", juce::dontSendNotification);
    chainEnableLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(chainEnableToggle);
    addAndMakeVisible(chainEnableLabel);
    chainEnableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "drumchainenabled", chainEnableToggle);

    chainStepsSlider.setSliderStyle(juce::Slider::IncDecButtons);
    chainStepsSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 20);
    chainStepsSlider.setRange(1, 8, 1);
    chainStepsSlider.setValue(4);
    chainStepsSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    addAndMakeVisible(chainStepsSlider);
    chainStepsLabel.setText("Steps", juce::dontSendNotification);
    chainStepsLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(chainStepsLabel);
    chainStepsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drumchainsteps", chainStepsSlider);
    chainStepsSlider.onValueChange = [this]() { resized(); };

    const char* chainStepParamIds[] = {
        "drumchainstep1", "drumchainstep2", "drumchainstep3", "drumchainstep4",
        "drumchainstep5", "drumchainstep6", "drumchainstep7", "drumchainstep8"
    };
    for (int i = 0; i < NUM_CHAIN_STEPS; ++i)
    {
        chainStepSliders[i].setSliderStyle(juce::Slider::IncDecButtons);
        chainStepSliders[i].setTextBoxStyle(juce::Slider::TextBoxLeft, false, 30, 30);
        chainStepSliders[i].setRange(1, 8, 1);
        chainStepSliders[i].setValue(1);
        chainStepSliders[i].setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
        chainStepSliders[i].setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff404040));
        addAndMakeVisible(chainStepSliders[i]);

        chainStepLabels[i].setText(juce::String(i + 1), juce::dontSendNotification);
        chainStepLabels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(chainStepLabels[i]);

        chainStepAttachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), chainStepParamIds[i], chainStepSliders[i]);
    }

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

    const int startY = 60;
    const int buttonSize = 40;
    const int verticalSpacing = 50;

    // Draw in reverse order: OHat at top, Kick at bottom
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        int visualRow = NUM_LANES - 1 - lane; // Reverse: lane 0 (Kick) at bottom
        int y = startY + visualRow * verticalSpacing;
        g.drawText(laneNames[lane], 10, y, 50, buttonSize, juce::Justification::centredRight);
    }

    // Highlight current drum step in grid
    if (currentStep >= 0 && currentStep < NUM_STEPS)
    {
        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        int x = 70 + currentStep * 50;
        int height = NUM_LANES * verticalSpacing;
        g.fillRect(x, startY, buttonSize, height);
    }

    // Highlight current chain step (if chain enabled)
    bool chainEnabled = audioProcessor.getValueTreeState().getRawParameterValue("drumchainenabled")->load() > 0.5f;
    if (chainEnabled)
    {
        int chainStep = audioProcessor.currentDrumChainStep;
        int numChainSteps = static_cast<int>(audioProcessor.getValueTreeState().getRawParameterValue("drumchainsteps")->load());
        if (chainStep >= 0 && chainStep < numChainSteps)
        {
            // Calculate chain step position (same layout as resized)
            const int dialY = startY + NUM_LANES * verticalSpacing + 10;
            const int knobSize = 60;
            const int labelHeight = 20;
            const int patternButtonHeight = 30;
            const int patternY = dialY + knobSize + labelHeight + 15;
            const int chainControlY = patternY + patternButtonHeight + 15;
            const int chainStepsY = chainControlY + 55;
            const int chainSliderWidth = 60;
            const int chainSliderHeight = 60;
            const int chainSliderSpacing = 90;
            const int chainLabelHeight = 20;

            // Center calculation (same as resized)
            const int totalChainWidth = numChainSteps * chainSliderSpacing - (chainSliderSpacing - chainSliderWidth);
            const int chainStepsStartX = (getWidth() - totalChainWidth) / 2;

            g.setColour(juce::Colours::orange.withAlpha(0.3f));
            g.fillRect(chainStepsStartX + chainStep * chainSliderSpacing - 5,
                       chainStepsY - chainLabelHeight - 10,
                       chainSliderWidth + 10,
                       chainSliderHeight + chainLabelHeight + 15);
        }
    }
}

void DrumTab::resized()
{
    // Top controls - just enable toggle
    const int controlY = 20;
    const int controlHeight = 25;

    enableToggle.setBounds(20, controlY, 25, controlHeight);
    enableLabel.setBounds(50, controlY, 60, controlHeight);

    // Step button grid
    const int startX = 70;
    const int startY = 60;
    const int buttonSize = 40;
    const int horizontalSpacing = 50;
    const int verticalSpacing = 50;

    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        int visualRow = NUM_LANES - 1 - lane; // Reverse: lane 0 (Kick) at bottom
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            stepButtons[lane][step].setBounds(
                startX + step * horizontalSpacing,
                startY + visualRow * verticalSpacing,
                buttonSize,
                buttonSize
            );
        }
    }

    // Control dials below grid (60px like synth tab)
    const int knobSize = 60;
    const int labelHeight = 20;
    const int dialY = startY + NUM_LANES * verticalSpacing + 10;
    const int dialSpacing = 80;

    // Master volume first
    globalVolumeSlider.setBounds(startX, dialY, knobSize, knobSize);
    globalVolumeLabel.setBounds(startX, dialY + knobSize, knobSize, labelHeight);

    // Per-lane volume sliders: Kick, Snare, CHat, OHat
    const char* laneLabels[NUM_LANES] = { "Kick", "Snare", "CHat", "OHat" };
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        laneVolumeSliders[lane].setBounds(startX + (lane + 1) * dialSpacing, dialY, knobSize, knobSize);
        laneVolumeLabels[lane].setBounds(startX + (lane + 1) * dialSpacing, dialY + knobSize, knobSize, labelHeight);
        laneVolumeLabels[lane].setText(laneLabels[lane], juce::dontSendNotification);
    }

    // Sidechain controls (after gap)
    const int sidechainX = startX + (NUM_LANES + 1) * dialSpacing + 40;

    sidechainMagnitudeSlider.setBounds(sidechainX, dialY, knobSize, knobSize);
    sidechainMagnitudeLabel.setBounds(sidechainX, dialY + knobSize, knobSize, labelHeight);

    sidechainLengthSlider.setBounds(sidechainX + dialSpacing, dialY, knobSize, knobSize);
    sidechainLengthLabel.setBounds(sidechainX + dialSpacing, dialY + knobSize, knobSize, labelHeight);

    // Pattern selection buttons (centered below dials)
    const int patternButtonWidth = 40;
    const int patternButtonHeight = 30;
    const int patternButtonSpacing = 50;
    const int totalPatternWidth = NUM_PATTERNS * patternButtonSpacing - (patternButtonSpacing - patternButtonWidth);
    const int patternY = dialY + knobSize + labelHeight + 15;
    const int patternStartX = (getWidth() - totalPatternWidth) / 2;

    for (int i = 0; i < NUM_PATTERNS; ++i)
    {
        patternButtons[i].setBounds(patternStartX + i * patternButtonSpacing, patternY, patternButtonWidth, patternButtonHeight);
    }

    // Chain sequencer (below pattern buttons)
    const int chainControlY = patternY + patternButtonHeight + 15;

    // Chain step sliders (same size as Progressions tab: 60x60, spacing 90)
    const int chainSliderWidth = 60;
    const int chainSliderHeight = 60;
    const int chainSliderSpacing = 90;
    const int chainLabelHeight = 20;
    int numActiveChainSteps = static_cast<int>(chainStepsSlider.getValue());

    // Center the chain steps
    const int totalChainWidth = numActiveChainSteps * chainSliderSpacing - (chainSliderSpacing - chainSliderWidth);
    const int chainStepsStartX = (getWidth() - totalChainWidth) / 2;
    const int chainStepsY = chainControlY + 55;

    // Center controls above chain steps
    const int controlsWidth = 200;
    const int controlsStartX = (getWidth() - controlsWidth) / 2;

    chainEnableToggle.setBounds(controlsStartX, chainControlY, 25, 25);
    chainEnableLabel.setBounds(controlsStartX + 28, chainControlY, 50, 25);
    chainStepsLabel.setBounds(controlsStartX + 90, chainControlY, 45, 25);
    chainStepsSlider.setBounds(controlsStartX + 140, chainControlY, 75, 25);

    for (int i = 0; i < NUM_CHAIN_STEPS; ++i)
    {
        bool isVisible = (i < numActiveChainSteps);
        chainStepSliders[i].setVisible(isVisible);
        chainStepLabels[i].setVisible(isVisible);

        if (isVisible)
        {
            chainStepLabels[i].setBounds(chainStepsStartX + i * chainSliderSpacing, chainStepsY - chainLabelHeight - 5, chainSliderWidth, chainLabelHeight);
            chainStepSliders[i].setBounds(chainStepsStartX + i * chainSliderSpacing, chainStepsY, chainSliderWidth, chainSliderHeight);
        }
    }
}

void DrumTab::timerCallback()
{
    int newStep = audioProcessor.currentDrumStep;
    int newPattern = audioProcessor.currentDrumPatternIndex;
    int newChainStep = audioProcessor.currentDrumChainStep;

    bool needsRepaint = false;

    if (newStep != currentStep)
    {
        currentStep = newStep;
        needsRepaint = true;
    }

    if (newPattern != lastPatternIndex)
    {
        lastPatternIndex = newPattern;
        updateButtonStates(); // Refresh step grid for new pattern
    }

    if (newChainStep != lastChainStep)
    {
        lastChainStep = newChainStep;
        needsRepaint = true;
    }

    if (needsRepaint)
        repaint();

    // Always update pattern buttons to show pending state
    updatePatternButtonStates();
}

void DrumTab::onStepButtonClicked(int lane, int step)
{
    bool isOn = stepButtons[lane][step].getToggleState();
    int patternIdx = audioProcessor.currentDrumPatternIndex;
    audioProcessor.drumPatterns[patternIdx][lane][step] = isOn ? 1 : 0;
}

void DrumTab::updateButtonStates()
{
    int patternIdx = audioProcessor.currentDrumPatternIndex;
    for (int lane = 0; lane < NUM_LANES; ++lane)
    {
        for (int step = 0; step < NUM_STEPS; ++step)
        {
            bool isOn = audioProcessor.drumPatterns[patternIdx][lane][step] != 0;
            stepButtons[lane][step].setToggleState(isOn, juce::dontSendNotification);
        }
    }
}

void DrumTab::onPatternButtonClicked(int patternIndex)
{
    audioProcessor.selectDrumPattern(patternIndex);
    updatePatternButtonStates();
}

void DrumTab::updatePatternButtonStates()
{
    int currentIdx = audioProcessor.currentDrumPatternIndex;
    int pendingIdx = audioProcessor.pendingDrumPatternIndex;

    for (int i = 0; i < NUM_PATTERNS; ++i)
    {
        bool isActive = (i == currentIdx);
        bool isPending = (i == pendingIdx);

        patternButtons[i].setToggleState(isActive, juce::dontSendNotification);

        // Different color for pending pattern
        if (isPending)
            patternButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff885500));
        else
            patternButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
    }
}
