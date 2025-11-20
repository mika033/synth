#include "PluginProcessor.h"
#include "FilterTab.h"

//==============================================================================
FilterTab::FilterTab(SnorkelSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Configure preset selector - load from JSON
    juce::StringArray presetNames = audioProcessor.getSynthPresetNames();
    for (int i = 0; i < presetNames.size(); ++i)
        presetSelector.addItem(presetNames[i], i + 1);

    if (presetNames.size() > 0)
        presetSelector.setSelectedId(1); // Select first preset by default

    presetSelector.onChange = [this]
    {
        int selectedIndex = presetSelector.getSelectedItemIndex();
        if (selectedIndex >= 0)
            audioProcessor.setCurrentProgram(selectedIndex);
    };
    addAndMakeVisible(presetSelector);
    presetLabel.setText("Synth presets", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(presetLabel);

    // Configure save preset button
    savePresetButton.setButtonText("Save");
    savePresetButton.onClick = [this]
    {
        juce::AlertWindow* w = new juce::AlertWindow("Save Preset", "Enter preset name:", juce::MessageBoxIconType::NoIcon);
        w->addTextEditor("presetName", "", "Preset name:");
        w->addButton("OK", 1);
        w->addButton("Cancel", 0);

        w->enterModalState(true, juce::ModalCallbackFunction::create([this, w](int result)
        {
            if (result == 1)
            {
                juce::String presetName = w->getTextEditorContents("presetName");
                if (presetName.isNotEmpty())
                {
                    // Save the preset (this updates the in-memory list immediately)
                    audioProcessor.saveSynthPresetToJSON(presetName);

                    // Refresh preset selector
                    presetSelector.clear();
                    juce::StringArray presetNames = audioProcessor.getSynthPresetNames();
                    for (int i = 0; i < presetNames.size(); ++i)
                        presetSelector.addItem(presetNames[i], i + 1);

                    // Select and load the newly saved preset (last one in the list)
                    int newPresetIndex = presetNames.size() - 1;  // Index in names array
                    presetSelector.setSelectedId(presetNames.size(), juce::dontSendNotification);
                    audioProcessor.setCurrentProgram(newPresetIndex);  // Load it explicitly
                }
            }
            delete w;
        }), true);
    };
    addAndMakeVisible(savePresetButton);

    // Configure cutoff slider
    cutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(cutoffSlider);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "cutoff", cutoffSlider);

    // Configure resonance slider
    resonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(resonanceSlider);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceLabel);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "resonance", resonanceSlider);

    // Configure envelope mod slider
    envModSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    envModSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(envModSlider);
    envModLabel.setText("Env Mod", juce::dontSendNotification);
    envModLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(envModLabel);
    envModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "envmod", envModSlider);

    // Configure accent slider
    accentSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    accentSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(accentSlider);
    accentLabel.setText("Accent", juce::dontSendNotification);
    accentLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentLabel);
    accentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "accent", accentSlider);

    // Configure Filter Feedback slider
    filterFeedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterFeedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterFeedbackSlider);
    filterFeedbackLabel.setText("Filter FB", juce::dontSendNotification);
    filterFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterFeedbackLabel);
    filterFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterfeedback", filterFeedbackSlider);

    // Configure Saturation Type selector
    saturationTypeSelector.addItem("Clean", 1);
    saturationTypeSelector.addItem("Warm", 2);
    saturationTypeSelector.addItem("Tube", 3);
    saturationTypeSelector.addItem("Hard", 4);
    saturationTypeSelector.addItem("Acid", 5);
    addAndMakeVisible(saturationTypeSelector);
    saturationTypeLabel.setText("Saturation", juce::dontSendNotification);
    saturationTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationTypeLabel);
    saturationTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "saturationtype", saturationTypeSelector);

    // Configure Filter ADSR sliders
    filterAttackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterAttackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterAttackSlider);
    filterAttackLabel.setText("Attack", juce::dontSendNotification);
    filterAttackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterAttackLabel);
    filterAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterattack", filterAttackSlider);

    filterDecaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterDecaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterDecaySlider);
    filterDecayLabel.setText("Decay", juce::dontSendNotification);
    filterDecayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterDecayLabel);
    filterDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterdecay", filterDecaySlider);

    filterSustainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterSustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterSustainSlider);
    filterSustainLabel.setText("Sustain", juce::dontSendNotification);
    filterSustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterSustainLabel);
    filterSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filtersustain", filterSustainSlider);

    filterReleaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterReleaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterReleaseSlider);
    filterReleaseLabel.setText("Release", juce::dontSendNotification);
    filterReleaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterReleaseLabel);
    filterReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterrelease", filterReleaseSlider);

    // Configure Delay Time selector
    delayTimeSelector.addItem("1/16", 1);
    delayTimeSelector.addItem("1/16.", 2);
    delayTimeSelector.addItem("1/16T", 3);
    delayTimeSelector.addItem("1/8", 4);
    delayTimeSelector.addItem("1/8.", 5);
    delayTimeSelector.addItem("1/8T", 6);
    delayTimeSelector.addItem("1/4", 7);
    delayTimeSelector.addItem("1/4.", 8);
    delayTimeSelector.addItem("1/4T", 9);
    delayTimeSelector.addItem("1/2", 10);
    delayTimeSelector.addItem("1/2.", 11);
    delayTimeSelector.addItem("1/1", 12);
    addAndMakeVisible(delayTimeSelector);
    delayTimeLabel.setText("Delay Time", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayTimeLabel);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "delaytime", delayTimeSelector);

    // Configure Delay Feedback slider
    delayFeedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayFeedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackLabel.setText("Delay FB", juce::dontSendNotification);
    delayFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayFeedbackLabel);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "delayfeedback", delayFeedbackSlider);

    // Configure Delay Mix slider
    delayMixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayMixSlider);
    delayMixLabel.setText("Delay Mix", juce::dontSendNotification);
    delayMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayMixLabel);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "delaymix", delayMixSlider);
}

FilterTab::~FilterTab()
{
}

//==============================================================================
void FilterTab::paint(juce::Graphics& g)
{
    // Draw panel background
    g.fillAll(juce::Colour(0xff2a2a2a));

    const int boxGap = 10;
    const int box1Y = 60;
    const int box1Height = 200;
    const int box2Y = box1Y + box1Height + boxGap;
    const int box2Height = 110;

    // Draw section backgrounds
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(20, box1Y, getWidth() - 40, box1Height, 10); // Filter & Filter ADSR
    g.fillRoundedRectangle(20, box2Y, getWidth() - 40, box2Height, 10); // Delay

    // Draw section labels
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);
    g.drawText("FILTER & FILTER ENVELOPE", 30, box1Y + 5, 250, 20, juce::Justification::left);
    g.drawText("DELAY", 30, box2Y + 5, 200, 20, juce::Justification::left);
}

void FilterTab::resized()
{
    const int knobSize = 60;
    const int labelHeight = 20;
    const int columnSpacing = 130;
    const int startX = 50;

    auto getColumnX = [&](int col) { return startX + col * columnSpacing; };

    // Preset controls at the top (Label -> Selector -> Save button)
    presetLabel.setBounds(getWidth() - 330, 15, 100, 20);
    presetSelector.setBounds(getWidth() - 220, 15, 130, 25);
    savePresetButton.setBounds(getWidth() - 85, 15, 50, 25);

    // BOX 1: FILTER & FILTER ENVELOPE
    // Row 1: Cutoff, Resonance, EnvMod, Accent, Filter FB, Saturation
    int box1Row1Y = 90;
    cutoffLabel.setBounds(getColumnX(0), box1Row1Y + knobSize, knobSize, labelHeight);
    cutoffSlider.setBounds(getColumnX(0), box1Row1Y, knobSize, knobSize);

    resonanceLabel.setBounds(getColumnX(1), box1Row1Y + knobSize, knobSize, labelHeight);
    resonanceSlider.setBounds(getColumnX(1), box1Row1Y, knobSize, knobSize);

    envModLabel.setBounds(getColumnX(2), box1Row1Y + knobSize, knobSize, labelHeight);
    envModSlider.setBounds(getColumnX(2), box1Row1Y, knobSize, knobSize);

    accentLabel.setBounds(getColumnX(3), box1Row1Y + knobSize, knobSize, labelHeight);
    accentSlider.setBounds(getColumnX(3), box1Row1Y, knobSize, knobSize);

    filterFeedbackLabel.setBounds(getColumnX(4), box1Row1Y + knobSize, knobSize, labelHeight);
    filterFeedbackSlider.setBounds(getColumnX(4), box1Row1Y, knobSize, knobSize);

    saturationTypeLabel.setBounds(getColumnX(5), box1Row1Y + knobSize, knobSize, labelHeight);
    saturationTypeSelector.setBounds(getColumnX(5) + 10, box1Row1Y + 25, 60, 25);

    // Row 2: Filter ADSR
    int box1Row2Y = box1Row1Y + 90;
    filterAttackLabel.setBounds(getColumnX(0), box1Row2Y + knobSize, knobSize, labelHeight);
    filterAttackSlider.setBounds(getColumnX(0), box1Row2Y, knobSize, knobSize);

    filterDecayLabel.setBounds(getColumnX(1), box1Row2Y + knobSize, knobSize, labelHeight);
    filterDecaySlider.setBounds(getColumnX(1), box1Row2Y, knobSize, knobSize);

    filterSustainLabel.setBounds(getColumnX(2), box1Row2Y + knobSize, knobSize, labelHeight);
    filterSustainSlider.setBounds(getColumnX(2), box1Row2Y, knobSize, knobSize);

    filterReleaseLabel.setBounds(getColumnX(3), box1Row2Y + knobSize, knobSize, labelHeight);
    filterReleaseSlider.setBounds(getColumnX(3), box1Row2Y, knobSize, knobSize);

    // BOX 2: DELAY
    int box2Row1Y = 300;
    delayTimeLabel.setBounds(getColumnX(0), box2Row1Y + knobSize, knobSize, labelHeight);
    delayTimeSelector.setBounds(getColumnX(0) + 10, box2Row1Y + 25, 60, 25);

    delayFeedbackLabel.setBounds(getColumnX(1), box2Row1Y + knobSize, knobSize, labelHeight);
    delayFeedbackSlider.setBounds(getColumnX(1), box2Row1Y, knobSize, knobSize);

    delayMixLabel.setBounds(getColumnX(2), box2Row1Y + knobSize, knobSize, labelHeight);
    delayMixSlider.setBounds(getColumnX(2), box2Row1Y, knobSize, knobSize);
}
