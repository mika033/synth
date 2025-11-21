#include "PluginProcessor.h"
#include "MelodySequencerTab.h"

//==============================================================================
MelodySequencerTab::MelodySequencerTab(SnorkelSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Configure preset selector - load from JSON
    juce::StringArray presetNames = audioProcessor.getSequencerPresetNames();
    for (int i = 0; i < presetNames.size(); ++i)
        presetSelector.addItem(presetNames[i], i + 1);

    if (presetNames.size() > 0)
    {
        presetSelector.setSelectedId(1); // Select first preset by default
        loadPreset(0); // Load the first preset
    }

    presetSelector.onChange = [this]
    {
        int selectedIndex = presetSelector.getSelectedItemIndex();
        loadPreset(selectedIndex);
    };
    addAndMakeVisible(presetSelector);
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(presetLabel);

    // Configure save preset button
    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this]
    {
        auto* w = new juce::AlertWindow("Save Preset", "Enter a name for the preset:", juce::MessageBoxIconType::NoIcon);
        w->addTextEditor("presetName", "", "Preset Name:");
        w->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
        w->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

        w->enterModalState(true, juce::ModalCallbackFunction::create([this, w](int result)
        {
            if (result == 1)
            {
                juce::String presetName = w->getTextEditorContents("presetName");
                if (presetName.isNotEmpty())
                {
                    // Save the preset (this updates the in-memory list immediately)
                    audioProcessor.saveSequencerPresetToJSON(presetName);

                    // Refresh preset selector
                    presetSelector.clear();
                    juce::StringArray presetNames = audioProcessor.getSequencerPresetNames();
                    for (int i = 0; i < presetNames.size(); ++i)
                        presetSelector.addItem(presetNames[i], i + 1);

                    // Select and load the newly saved preset (last one in the list)
                    int newPresetIndex = presetNames.size() - 1;  // Index in names array
                    presetSelector.setSelectedId(presetNames.size(), juce::dontSendNotification);
                    loadPreset(newPresetIndex);  // Load it explicitly
                }
            }
            delete w;
        }), true);
    };
    addAndMakeVisible(savePresetButton);

    // Configure Algorithm selector - dynamically load configs from JSON
    algoSelector.addItem("True Rand", 1);
    algoSelector.addItem("Weighted", 2);

    // Load config names from JSON
    auto* configObj = audioProcessor.randomizationConfigJSON.getDynamicObject();
    if (configObj != nullptr)
    {
        auto* configsArray = configObj->getProperty("weightedConfigs").getArray();
        if (configsArray != nullptr)
        {
            for (int i = 0; i < configsArray->size(); ++i)
            {
                auto* config = (*configsArray)[i].getDynamicObject();
                if (config != nullptr)
                {
                    juce::String configName = config->getProperty("name").toString();
                    if (configName.isNotEmpty())
                    {
                        algoSelector.addItem(configName, 3 + i); // Start at ID 3
                    }
                }
            }
        }
    }

    algoSelector.setSelectedId(2); // Default to Weighted
    addAndMakeVisible(algoSelector);

    algoLabel.setText("Algo", juce::dontSendNotification);
    algoLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(algoLabel);

    // Configure Random button
    randomButton.setButtonText("Random");
    randomButton.onClick = [this]() { onRandomClicked(); };
    addAndMakeVisible(randomButton);

    // Configure Mutate button
    mutateButton.setButtonText("Mutate");
    mutateButton.onClick = [this]() { onMutateClicked(); };
    addAndMakeVisible(mutateButton);

    // Configure enable toggle
    enableToggle.setButtonText("Enabled");
    addAndMakeVisible(enableToggle);

    // When seq is enabled, disable arpeggiator
    enableToggle.onClick = [this]()
    {
        if (enableToggle.getToggleState())
        {
            // Disable arpeggiator when seq is enabled
            auto* arpParam = audioProcessor.getValueTreeState().getParameter("arponoff");
            if (arpParam != nullptr)
                arpParam->setValueNotifyingHost(0.0f);
        }
    };

    // Configure Steps slider (same style as Progression)
    stepsSlider.setSliderStyle(juce::Slider::IncDecButtons);
    stepsSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 40, 20);
    stepsSlider.setRange(1, 16, 1);
    stepsSlider.setValue(16);
    stepsSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    addAndMakeVisible(stepsSlider);

    stepsLabel.setText("Steps", juce::dontSendNotification);
    stepsLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(stepsLabel);

    // Configure Rate selector
    rateSelector.addItem("1/32", 1);
    rateSelector.addItem("1/32.", 2);
    rateSelector.addItem("1/16", 3);
    rateSelector.addItem("1/16.", 4);
    rateSelector.addItem("1/16T", 5);
    rateSelector.addItem("1/8", 6);
    rateSelector.addItem("1/8.", 7);
    rateSelector.addItem("1/8T", 8);
    rateSelector.addItem("1/4", 9);
    rateSelector.addItem("1/4.", 10);
    rateSelector.addItem("1/4T", 11);
    rateSelector.addItem("1/2", 12);
    rateSelector.addItem("1/2.", 13);
    rateSelector.addItem("1/1", 14);
    rateSelector.setSelectedId(3); // Default to 1/16
    addAndMakeVisible(rateSelector);

    rateLabel.setText("Rate", juce::dontSendNotification);
    rateLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(rateLabel);

    // Create 16x8 button grid
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        for (int degree = 0; degree < NUM_SCALE_DEGREES; ++degree)
        {
            auto& button = stepButtons[step][degree];

            // Set step and degree for this button
            button.step = step;
            button.degree = degree;

            // Button text will be set to octave value (updated in updateOctaveDisplay)
            button.setButtonText("");

            // Make buttons toggleable
            button.setClickingTogglesState(true);

            // Style buttons
            button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff404040));
            button.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xff00aa00));
            button.setColour(juce::TextButton::textColourOffId, juce::Colours::grey);
            button.setColour(juce::TextButton::textColourOnId, juce::Colours::white);

            // Add click handler for normal clicks
            button.onClick = [this, step, degree]()
            {
                onStepButtonClicked(step, degree);
            };

            // Add octave adjust handler for clicks on already-active buttons
            button.onOctaveAdjust = [this](int s, int d, bool isIncrease)
            {
                if (isIncrease)
                    onOctaveUpClicked(s, d);
                else
                    onOctaveDownClicked(s, d);
            };

            // Add deactivate handler for middle-third clicks
            button.onDeactivate = [this, step, degree](int s)
            {
                // Deactivate this note (clear the bit)
                stepButtons[s][degree].setToggleState(false, juce::dontSendNotification);
                audioProcessor.sequencerPattern[s] &= static_cast<uint8_t>(~(1 << degree));
                updateOctaveDisplayForStep(s);
            };

            addAndMakeVisible(button);
        }
    }

    // Create accent modulation fill bars for each step
    const char* accentParamIds[] = {
        "seqcutoff1", "seqcutoff2", "seqcutoff3", "seqcutoff4",
        "seqcutoff5", "seqcutoff6", "seqcutoff7", "seqcutoff8",
        "seqcutoff9", "seqcutoff10", "seqcutoff11", "seqcutoff12",
        "seqcutoff13", "seqcutoff14", "seqcutoff15", "seqcutoff16"
    };

    for (int step = 0; step < NUM_STEPS; ++step)
    {
        accentSliders[step].setSliderStyle(juce::Slider::LinearBarVertical);
        accentSliders[step].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        accentSliders[step].setRange(-1.0, 1.0, 0.01);
        accentSliders[step].setValue(0.0);
        accentSliders[step].setLookAndFeel(&fillBarLookAndFeel);
        addAndMakeVisible(accentSliders[step]);

        accentAttachments[step] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.getValueTreeState(), accentParamIds[step], accentSliders[step]);
    }

    // Create accent control dials
    auto configureRotary = [](juce::Slider& slider) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };

    configureRotary(accentVolSlider);
    addAndMakeVisible(accentVolSlider);
    accentVolLabel.setText("Volume", juce::dontSendNotification);
    accentVolLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentVolLabel);
    accentVolAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "seqaccentvol", accentVolSlider);

    configureRotary(accentCutoffSlider);
    addAndMakeVisible(accentCutoffSlider);
    accentCutoffLabel.setText("Cutoff", juce::dontSendNotification);
    accentCutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentCutoffLabel);
    accentCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "seqaccentcutoff", accentCutoffSlider);

    configureRotary(accentResSlider);
    addAndMakeVisible(accentResSlider);
    accentResLabel.setText("Resonance", juce::dontSendNotification);
    accentResLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentResLabel);
    accentResAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "seqaccentres", accentResSlider);

    configureRotary(accentDecaySlider);
    addAndMakeVisible(accentDecaySlider);
    accentDecayLabel.setText("Decay", juce::dontSendNotification);
    accentDecayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentDecayLabel);
    accentDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "seqaccentdecay", accentDecaySlider);

    configureRotary(accentDriveSlider);
    addAndMakeVisible(accentDriveSlider);
    accentDriveLabel.setText("Drive", juce::dontSendNotification);
    accentDriveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentDriveLabel);
    accentDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "seqaccentdrive", accentDriveSlider);

    // Attach to parameters
    enableAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "seqenabled", enableToggle);
    stepsAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "seqsteps", stepsSlider);
    rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "seqrate", rateSelector);

    // Load initial button states from processor
    updateButtonStates();
    updateOctaveDisplay();

    // Start timer to update current step visualization
    startTimer(50); // Update at 20Hz
}

MelodySequencerTab::~MelodySequencerTab()
{
    stopTimer();

    // Clean up custom LookAndFeel from all accent sliders
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        accentSliders[step].setLookAndFeel(nullptr);
    }
}

//==============================================================================
void MelodySequencerTab::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw labels for scale degrees on the left
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12.0f);
    const char* degreeLabels[] = {"8", "7", "6", "5", "4", "3", "2", "1"};
    for (int i = 0; i < 8; ++i)
    {
        int y = 80 + i * 50;
        g.drawText(degreeLabels[i], 30, y, 30, 40, juce::Justification::centredRight);
    }

    // Highlight current step (extends to cover grid and accent row)
    if (currentStep >= 0 && currentStep < NUM_STEPS)
    {
        g.setColour(juce::Colours::orange.withAlpha(0.3f));
        int x = 70 + currentStep * 50;
        // Extend height to cover: grid (400) + accent fill bar row (55) = 455
        g.fillRect(x, 80, 40, 455);
    }

    // (Cutoff label removed - now using visual fill bars instead)
}

void MelodySequencerTab::resized()
{
    const int startX = 80;
    const int topRowY = 20;
    const int elementHeight = 25;

    // First row of controls - all aligned at same y and height, arranged sequentially
    int x = 20;
    const int gap = 5;

    // Enable toggle
    enableToggle.setBounds(x, topRowY, 60, elementHeight);
    x += 60 + gap;

    // Algorithm
    algoLabel.setBounds(x, topRowY, 30, elementHeight);
    x += 30 + gap;
    algoSelector.setBounds(x, topRowY, 80, elementHeight);
    x += 80 + gap;

    // Random and Mutate buttons
    randomButton.setBounds(x, topRowY, 65, elementHeight);
    x += 65 + gap;
    mutateButton.setBounds(x, topRowY, 65, elementHeight);
    x += 65 + gap + 10; // Extra spacing before steps/rate

    // Steps slider (same style as Progression - text box with +/- buttons)
    stepsLabel.setBounds(x, topRowY, 60, elementHeight);
    x += 65;
    stepsSlider.setBounds(x, topRowY, 60, 30);  // Height 30 like Progression
    x += 60 + gap + 5;

    // Rate selector
    rateLabel.setBounds(x, topRowY, 30, elementHeight);
    x += 35;
    rateSelector.setBounds(x, topRowY, 60, elementHeight);

    // Preset selector and save button at the far right
    presetLabel.setBounds(getWidth() - 280, topRowY, 60, elementHeight);
    presetSelector.setBounds(getWidth() - 215, topRowY, 130, elementHeight);
    savePresetButton.setBounds(getWidth() - 75, topRowY, 60, elementHeight);

    // 16x8 button grid
    const int buttonWidth = 40;
    const int buttonHeight = 40;
    const int horizontalSpacing = 50;
    const int verticalSpacing = 50;
    const int startY = 80;

    for (int step = 0; step < NUM_STEPS; ++step)
    {
        for (int degree = 0; degree < NUM_SCALE_DEGREES; ++degree)
        {
            // Buttons go from top (degree 7/octave) to bottom (degree 1)
            int reverseDegree = NUM_SCALE_DEGREES - 1 - degree;

            stepButtons[step][degree].setBounds(
                startX + step * horizontalSpacing,
                startY + reverseDegree * verticalSpacing,
                buttonWidth,
                buttonHeight
            );
        }
    }

    // Accent modulation fill bar row (directly below the button grid, same spacing as grid)
    const int accentRowY = startY + (NUM_SCALE_DEGREES * verticalSpacing);
    const int fillBarWidth = 40;  // Same as step buttons
    const int fillBarHeight = 40; // Same height as step buttons

    for (int step = 0; step < NUM_STEPS; ++step)
    {
        accentSliders[step].setBounds(
            startX + step * horizontalSpacing,
            accentRowY,
            fillBarWidth,
            fillBarHeight
        );
    }

    // Accent control dials (below accent row)
    const int knobSize = 50;
    const int labelHeight = 20;
    const int controlRowY = accentRowY + fillBarHeight + 5;
    const int controlSpacing = 70;
    const int controlStartX = startX;

    accentVolSlider.setBounds(controlStartX, controlRowY, knobSize, knobSize);
    accentVolLabel.setBounds(controlStartX, controlRowY + knobSize, knobSize, labelHeight);

    accentCutoffSlider.setBounds(controlStartX + controlSpacing, controlRowY, knobSize, knobSize);
    accentCutoffLabel.setBounds(controlStartX + controlSpacing, controlRowY + knobSize, knobSize, labelHeight);

    accentResSlider.setBounds(controlStartX + controlSpacing * 2, controlRowY, knobSize, knobSize);
    accentResLabel.setBounds(controlStartX + controlSpacing * 2, controlRowY + knobSize, knobSize, labelHeight);

    accentDecaySlider.setBounds(controlStartX + controlSpacing * 3, controlRowY, knobSize, knobSize);
    accentDecayLabel.setBounds(controlStartX + controlSpacing * 3, controlRowY + knobSize, knobSize, labelHeight);

    accentDriveSlider.setBounds(controlStartX + controlSpacing * 4, controlRowY, knobSize, knobSize);
    accentDriveLabel.setBounds(controlStartX + controlSpacing * 4, controlRowY + knobSize, knobSize, labelHeight);
}

void MelodySequencerTab::onStepButtonClicked(int step, int degree)
{
    // Update the sequencer pattern bitmask
    bool isOn = stepButtons[step][degree].getToggleState();
    if (isOn)
    {
        // Set the bit for this degree
        audioProcessor.sequencerPattern[step] |= static_cast<uint8_t>(1 << degree);
    }
    else
    {
        // Clear the bit for this degree
        audioProcessor.sequencerPattern[step] &= static_cast<uint8_t>(~(1 << degree));
    }

    // Update octave display for this step
    updateOctaveDisplayForStep(step);
}

void MelodySequencerTab::updateButtonStates()
{
    // Update button states from processor (bitmask)
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        uint8_t pattern = audioProcessor.sequencerPattern[step];
        for (int d = 0; d < NUM_SCALE_DEGREES; ++d)
        {
            bool isActive = (pattern & (1 << d)) != 0;
            stepButtons[step][d].setToggleState(isActive, juce::dontSendNotification);
        }
    }

    // Update octave display for all steps
    updateOctaveDisplay();
}

void MelodySequencerTab::timerCallback()
{
    // Update current step from processor
    int newStep = audioProcessor.currentSeqStep;
    if (newStep != currentStep)
    {
        currentStep = newStep;
        repaint();
    }

    // Update octave display periodically
    updateOctaveDisplay();
}

void MelodySequencerTab::loadPreset(int presetIndex)
{
    // Load preset from JSON via processor
    juce::StringArray presetNames = audioProcessor.getSequencerPresetNames();
    if (presetIndex < 0 || presetIndex >= presetNames.size())
        return;

    // Account for the "** User presets **" divider in the preset list
    int actualPresetIndex = presetIndex;

    if (audioProcessor.numSystemSequencerPresets > 0)
    {
        if (presetIndex == audioProcessor.numSystemSequencerPresets)
        {
            // User selected the divider itself - don't load anything
            return;
        }
        else if (presetIndex > audioProcessor.numSystemSequencerPresets)
        {
            // User selected a user preset - adjust index to skip divider
            actualPresetIndex = presetIndex - 1;
        }
    }

    // Get the preset data from JSON
    auto obj = audioProcessor.sequencerPresetsJSON.getDynamicObject();
    if (obj == nullptr)
        return;

    const juce::Array<juce::var>* presetsArray = obj->getProperty("presets").getArray();
    if (presetsArray == nullptr)
        return;

    auto* presetObj = (*presetsArray)[actualPresetIndex].getDynamicObject();
    if (presetObj == nullptr)
        return;

    const juce::Array<juce::var>* patternArray = presetObj->getProperty("pattern").getArray();
    if (patternArray == nullptr)
        return;

    // Check preset version (version 2 = new multi-note format)
    int version = static_cast<int>(presetObj->getProperty("version"));

    if (version >= 2)
    {
        // New format: pattern is already bitmask, octave is 2D array
        for (int step = 0; step < NUM_STEPS && step < patternArray->size(); ++step)
        {
            audioProcessor.sequencerPattern[step] = static_cast<uint8_t>(static_cast<int>((*patternArray)[step]));
        }

        // Load 2D octave array
        const juce::Array<juce::var>* octaveArray = presetObj->getProperty("octave").getArray();
        if (octaveArray != nullptr)
        {
            for (int step = 0; step < NUM_STEPS && step < octaveArray->size(); ++step)
            {
                const juce::Array<juce::var>* stepOctaves = (*octaveArray)[step].getArray();
                if (stepOctaves != nullptr)
                {
                    for (int degree = 0; degree < NUM_SCALE_DEGREES && degree < stepOctaves->size(); ++degree)
                    {
                        audioProcessor.sequencerOctave[step][degree] = static_cast<int8_t>(static_cast<int>((*stepOctaves)[degree]));
                    }
                }
            }
        }

        // Load accent values per step
        const juce::Array<juce::var>* accentsArray = presetObj->getProperty("accents").getArray();
        if (accentsArray != nullptr)
        {
            const char* accentParamIds[] = {
                "seqcutoff1", "seqcutoff2", "seqcutoff3", "seqcutoff4",
                "seqcutoff5", "seqcutoff6", "seqcutoff7", "seqcutoff8",
                "seqcutoff9", "seqcutoff10", "seqcutoff11", "seqcutoff12",
                "seqcutoff13", "seqcutoff14", "seqcutoff15", "seqcutoff16"
            };
            for (int step = 0; step < NUM_STEPS && step < accentsArray->size(); ++step)
            {
                auto* param = audioProcessor.getValueTreeState().getParameter(accentParamIds[step]);
                if (param != nullptr)
                {
                    float val = static_cast<float>((*accentsArray)[step]);
                    param->setValueNotifyingHost(audioProcessor.getValueTreeState().getParameterRange(accentParamIds[step]).convertTo0to1(val));
                }
            }
        }

        // Load accent dial settings
        auto* accentVolParam = audioProcessor.getValueTreeState().getParameter("seqaccentvol");
        auto* accentCutoffParam = audioProcessor.getValueTreeState().getParameter("seqaccentcutoff");
        auto* accentResParam = audioProcessor.getValueTreeState().getParameter("seqaccentres");
        auto* accentDecayParam = audioProcessor.getValueTreeState().getParameter("seqaccentdecay");
        auto* accentDriveParam = audioProcessor.getValueTreeState().getParameter("seqaccentdrive");

        if (accentVolParam && presetObj->hasProperty("accentVol"))
            accentVolParam->setValueNotifyingHost(audioProcessor.getValueTreeState().getParameterRange("seqaccentvol").convertTo0to1(static_cast<float>(presetObj->getProperty("accentVol"))));
        if (accentCutoffParam && presetObj->hasProperty("accentCutoff"))
            accentCutoffParam->setValueNotifyingHost(audioProcessor.getValueTreeState().getParameterRange("seqaccentcutoff").convertTo0to1(static_cast<float>(presetObj->getProperty("accentCutoff"))));
        if (accentResParam && presetObj->hasProperty("accentRes"))
            accentResParam->setValueNotifyingHost(audioProcessor.getValueTreeState().getParameterRange("seqaccentres").convertTo0to1(static_cast<float>(presetObj->getProperty("accentRes"))));
        if (accentDecayParam && presetObj->hasProperty("accentDecay"))
            accentDecayParam->setValueNotifyingHost(audioProcessor.getValueTreeState().getParameterRange("seqaccentdecay").convertTo0to1(static_cast<float>(presetObj->getProperty("accentDecay"))));
        if (accentDriveParam && presetObj->hasProperty("accentDrive"))
            accentDriveParam->setValueNotifyingHost(audioProcessor.getValueTreeState().getParameterRange("seqaccentdrive").convertTo0to1(static_cast<float>(presetObj->getProperty("accentDrive"))));
    }
    else
    {
        // Old format: pattern is single degree per step, octave is 1D array
        for (int step = 0; step < NUM_STEPS && step < patternArray->size(); ++step)
        {
            int degree = static_cast<int>((*patternArray)[step]);
            if (degree >= 0 && degree < 8)
                audioProcessor.sequencerPattern[step] = static_cast<uint8_t>(1 << degree);
            else
                audioProcessor.sequencerPattern[step] = 0;

            // Reset octave values to 0 for old format
            for (int d = 0; d < NUM_SCALE_DEGREES; ++d)
                audioProcessor.sequencerOctave[step][d] = 0;
        }

        // Load 1D octave array (apply to all degrees in step)
        const juce::Array<juce::var>* octaveArray = presetObj->getProperty("octave").getArray();
        if (octaveArray != nullptr)
        {
            for (int step = 0; step < NUM_STEPS && step < octaveArray->size(); ++step)
            {
                int octaveValue = static_cast<int>((*octaveArray)[step]);
                // Apply octave to all degrees in this step
                for (int d = 0; d < NUM_SCALE_DEGREES; ++d)
                    audioProcessor.sequencerOctave[step][d] = static_cast<int8_t>(octaveValue);
            }
        }
    }

    // Update button states to reflect loaded pattern
    updateButtonStates();
    updateOctaveDisplay();
}

void MelodySequencerTab::updateOctaveDisplay()
{
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        updateOctaveDisplayForStep(step);
    }
}

void MelodySequencerTab::updateOctaveDisplayForStep(int step)
{
    if (step < 0 || step >= NUM_STEPS)
        return;

    // Update all buttons in this step column with their individual octave values
    for (int degree = 0; degree < NUM_SCALE_DEGREES; ++degree)
    {
        if (stepButtons[step][degree].getToggleState())
        {
            // Get per-note octave value
            int octaveValue = audioProcessor.sequencerOctave[step][degree];

            // Format octave value for display
            juce::String octaveText;
            if (octaveValue > 0)
                octaveText = "+" + juce::String(octaveValue);
            else if (octaveValue < 0)
                octaveText = juce::String(octaveValue);
            else
                octaveText = ""; // Don't show 0

            stepButtons[step][degree].setButtonText(octaveText);
        }
        else
        {
            stepButtons[step][degree].setButtonText("");
        }
    }
}

void MelodySequencerTab::onOctaveUpClicked(int step, int degree)
{
    if (step < 0 || step >= NUM_STEPS || degree < 0 || degree >= NUM_SCALE_DEGREES)
        return;

    // Increment per-note octave (max +2)
    int8_t& octave = audioProcessor.sequencerOctave[step][degree];
    if (octave < 2)
        octave++;

    updateOctaveDisplayForStep(step);
}

void MelodySequencerTab::onOctaveDownClicked(int step, int degree)
{
    if (step < 0 || step >= NUM_STEPS || degree < 0 || degree >= NUM_SCALE_DEGREES)
        return;

    // Decrement per-note octave (min -2)
    int8_t& octave = audioProcessor.sequencerOctave[step][degree];
    if (octave > -2)
        octave--;

    updateOctaveDisplayForStep(step);
}

void MelodySequencerTab::onRandomClicked()
{
    int selectedAlgo = algoSelector.getSelectedId();

    if (selectedAlgo == 1) // True Rand
    {
        generateTrueRandom();
    }
    else if (selectedAlgo == 2) // Weighted (random)
    {
        generateWeightedRandom(-1); // -1 = random config
    }
    else if (selectedAlgo >= 3) // Specific configs (dynamically loaded)
    {
        int configIndex = selectedAlgo - 3; // 3->0, 4->1, 5->2, etc.
        generateWeightedRandom(configIndex);
    }
    else
    {
        // Fallback to true random
        generateTrueRandom();
    }

    updateButtonStates();
}

void MelodySequencerTab::generateTrueRandom()
{
    juce::Random random;

    // Clear all steps first
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        audioProcessor.sequencerPattern[step] = 0;
    }

    // Randomly decide how many steps to fill (between 4 and 16)
    int numActiveSteps = random.nextInt(juce::Range<int>(4, 17));

    // Generate random pattern (single note per step for random)
    for (int i = 0; i < numActiveSteps; ++i)
    {
        int step = random.nextInt(NUM_STEPS);
        int degree = random.nextInt(NUM_SCALE_DEGREES); // Random scale degree 0-7
        audioProcessor.sequencerPattern[step] = static_cast<uint8_t>(1 << degree);
    }
}

void MelodySequencerTab::generateWeightedRandom(int specificConfigIndex)
{
    juce::Random random;

    // Clear all steps first
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        audioProcessor.sequencerPattern[step] = 0;
    }

    // Load weights from JSON config
    auto* configObj = audioProcessor.randomizationConfigJSON.getDynamicObject();
    if (configObj == nullptr)
    {
        // Fallback to true random if config not loaded
        generateTrueRandom();
        return;
    }

    // Get the array of weighted configs
    auto* configsArray = configObj->getProperty("weightedConfigs").getArray();
    if (configsArray == nullptr || configsArray->size() == 0)
    {
        generateTrueRandom();
        return;
    }

    // Select config: either specific index or random
    int selectedConfigIndex;
    if (specificConfigIndex >= 0 && specificConfigIndex < configsArray->size())
    {
        selectedConfigIndex = specificConfigIndex; // Use specific config
    }
    else
    {
        selectedConfigIndex = random.nextInt(configsArray->size()); // Random config
    }

    auto* weightedObj = (*configsArray)[selectedConfigIndex].getDynamicObject();

    if (weightedObj == nullptr)
    {
        generateTrueRandom();
        return;
    }

    // Get the config name and show it to the user
    juce::String configName = weightedObj->getProperty("name").toString();
    if (configName.isNotEmpty())
    {
        audioProcessor.showEditorMessage("Weighted: " + configName);
    }

    // Get first step root probability (checked FIRST)
    float firstStepRootProbability = static_cast<float>(weightedObj->getProperty("firstStepRootProbability"));
    if (firstStepRootProbability <= 0.0f || firstStepRootProbability > 1.0f)
        firstStepRootProbability = 0.9f; // Default to 90%

    // Get step weights (now direct percentages 0-100)
    auto* stepWeightsObj = weightedObj->getProperty("stepWeights").getDynamicObject();
    int stepWeights[16] = {80, 10, 40, 10, 70, 10, 40, 10, 75, 10, 40, 10, 70, 10, 40, 10}; // Default percentages

    if (stepWeightsObj != nullptr)
    {
        for (int i = 0; i < 16; ++i)
        {
            stepWeights[i] = static_cast<int>(stepWeightsObj->getProperty(juce::String(i + 1)));
        }
    }

    // Get degree weights (now numbered 1-8 to match scale notation)
    auto* degreeWeightsObj = weightedObj->getProperty("degreeWeights").getDynamicObject();
    int degreeWeights[8] = {40, 10, 15, 20, 30, 15, 15, 25}; // Default weights
    int totalDegreeWeight = 0;

    if (degreeWeightsObj != nullptr)
    {
        for (int i = 0; i < 8; ++i)
        {
            // Read as 1-8 from JSON
            degreeWeights[i] = static_cast<int>(degreeWeightsObj->getProperty(juce::String(i + 1)));
            totalDegreeWeight += degreeWeights[i];
        }
    }

    // Get octave weights
    auto* octaveWeightsObj = weightedObj->getProperty("octaveWeights").getDynamicObject();
    int octaveWeights[3] = {10, 80, 10}; // Default weights for -1, 0, +1
    int totalOctaveWeight = 0;

    if (octaveWeightsObj != nullptr)
    {
        octaveWeights[0] = static_cast<int>(octaveWeightsObj->getProperty("-1"));
        octaveWeights[1] = static_cast<int>(octaveWeightsObj->getProperty("0"));
        octaveWeights[2] = static_cast<int>(octaveWeightsObj->getProperty("1"));
        totalOctaveWeight = octaveWeights[0] + octaveWeights[1] + octaveWeights[2];
    }
    else
    {
        totalOctaveWeight = 100;
    }

    // Octave parameter IDs
    const char* octaveParamIds[] = {
        "seqoctave1", "seqoctave2", "seqoctave3", "seqoctave4",
        "seqoctave5", "seqoctave6", "seqoctave7", "seqoctave8",
        "seqoctave9", "seqoctave10", "seqoctave11", "seqoctave12",
        "seqoctave13", "seqoctave14", "seqoctave15", "seqoctave16"
    };

    // FIRST: Handle step 1 with firstStepRootProbability
    bool step0IsRoot = false;
    if (random.nextFloat() < firstStepRootProbability)
    {
        // Force step 0 to be root (bit 0 = degree 0)
        audioProcessor.sequencerPattern[0] = 1;
        step0IsRoot = true;

        // For root on step 1: octave is always 0 or -1 (50/50)
        int selectedOctave = random.nextBool() ? 0 : -1;

        auto* param = audioProcessor.getValueTreeState().getParameter(octaveParamIds[0]);
        if (param != nullptr)
        {
            param->setValueNotifyingHost(
                audioProcessor.getValueTreeState().getParameterRange(octaveParamIds[0])
                    .convertTo0to1(static_cast<float>(selectedOctave))
            );
        }
    }

    // Generate weighted pattern for all 16 steps
    for (int step = 0; step < NUM_STEPS; ++step)
    {
        // Skip step 0 if it was already set as root
        if (step == 0 && step0IsRoot)
            continue;

        // stepWeights are now direct percentages (0-100)
        float stepProbability = stepWeights[step] / 100.0f;

        // Roll to see if this step gets a note
        if (random.nextFloat() < stepProbability)
        {
            // Pick a weighted random degree using cumulative distribution
            int degreeRoll = random.nextInt(totalDegreeWeight);
            int cumulativeDegree = 0;
            int selectedDegree = -1;

            for (int degree = 0; degree < NUM_SCALE_DEGREES; ++degree)
            {
                cumulativeDegree += degreeWeights[degree];
                if (degreeRoll < cumulativeDegree)
                {
                    selectedDegree = degree; // Stored as 0-7 internally
                    break;
                }
            }

            if (selectedDegree != -1)
            {
                audioProcessor.sequencerPattern[step] = static_cast<uint8_t>(1 << selectedDegree);

                // Select octave based on degree
                int selectedOctave = 0;

                if (selectedDegree == 7) // Degree 8 (octave) - always use octave 0
                {
                    selectedOctave = 0;
                }
                else if (selectedDegree == 0) // Degree 1 (root) - always 0 or -1 (50/50)
                {
                    selectedOctave = random.nextBool() ? 0 : -1;
                }
                else // All other degrees - use weighted random
                {
                    int octaveRoll = random.nextInt(totalOctaveWeight);
                    int cumulativeOctave = 0;

                    for (int octIdx = 0; octIdx < 3; ++octIdx)
                    {
                        cumulativeOctave += octaveWeights[octIdx];
                        if (octaveRoll < cumulativeOctave)
                        {
                            selectedOctave = octIdx - 1; // Map 0,1,2 to -1,0,+1
                            break;
                        }
                    }
                }

                // Set the octave parameter for this step
                auto* param = audioProcessor.getValueTreeState().getParameter(octaveParamIds[step]);
                if (param != nullptr)
                {
                    param->setValueNotifyingHost(
                        audioProcessor.getValueTreeState().getParameterRange(octaveParamIds[step])
                            .convertTo0to1(static_cast<float>(selectedOctave))
                    );
                }
            }
        }
    }
}

void MelodySequencerTab::onMutateClicked()
{
    juce::Random random;

    // Pick a random step to mutate
    int stepToMutate = random.nextInt(NUM_STEPS);

    // 50% chance to either change the degree or toggle on/off
    if (random.nextBool())
    {
        // Change to a random degree (or empty)
        if (random.nextFloat() < 0.2f) // 20% chance to make it empty
        {
            audioProcessor.sequencerPattern[stepToMutate] = 0;
        }
        else
        {
            int degree = random.nextInt(NUM_SCALE_DEGREES);
            audioProcessor.sequencerPattern[stepToMutate] = static_cast<uint8_t>(1 << degree);
        }
    }
    else
    {
        // Toggle: if empty make it random, if has value make it empty
        if (audioProcessor.sequencerPattern[stepToMutate] == 0)
        {
            int degree = random.nextInt(NUM_SCALE_DEGREES);
            audioProcessor.sequencerPattern[stepToMutate] = static_cast<uint8_t>(1 << degree);
        }
        else
        {
            audioProcessor.sequencerPattern[stepToMutate] = 0;
        }
    }

    // Update UI to reflect the mutated pattern
    updateButtonStates();
}
