#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "OscTab.h"

//==============================================================================
OscTab::OscTab(SnorkelSynthAudioProcessor& p, SnorkelSynthAudioProcessorEditor& e)
    : audioProcessor(p), editor(e), oscTabs(juce::TabbedButtonBar::TabsAtTop)
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

    // Helper lambda to configure a rotary slider
    auto configureRotary = [](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    };

    auto configureLabel = [](juce::Label& label, const juce::String& text)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
    };

    // ========== OSCILLATOR 1 ==========
    configureRotary(osc1WaveSlider);
    osc1WaveSlider.setDoubleClickReturnValue(true, 0.5); // Default: Sawtooth
    addAndMakeVisible(osc1WaveSlider);
    configureLabel(osc1WaveLabel, "Wave");
    addAndMakeVisible(osc1WaveLabel);
    osc1WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc1wave", osc1WaveSlider);
    osc1WaveSlider.onValueChange = [this]() { updateFeedback("OSC 1 Wave", osc1WaveSlider.getValue()); };

    configureRotary(osc1CoarseSlider);
    osc1CoarseSlider.setDoubleClickReturnValue(true, 0); // Default: 0 semitones
    addAndMakeVisible(osc1CoarseSlider);
    configureLabel(osc1CoarseLabel, "Coarse");
    addAndMakeVisible(osc1CoarseLabel);
    osc1CoarseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc1coarse", osc1CoarseSlider);
    osc1CoarseSlider.onValueChange = [this]() { updateFeedback("OSC 1 Coarse", osc1CoarseSlider.getValue(), " semitones"); };

    configureRotary(osc1FineSlider);
    osc1FineSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 cents
    addAndMakeVisible(osc1FineSlider);
    configureLabel(osc1FineLabel, "Fine");
    addAndMakeVisible(osc1FineLabel);
    osc1FineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc1fine", osc1FineSlider);
    osc1FineSlider.onValueChange = [this]() { updateFeedback("OSC 1 Fine", osc1FineSlider.getValue(), " cents"); };

    configureRotary(osc1MixSlider);
    osc1MixSlider.setDoubleClickReturnValue(true, 0.7); // Default: 0.7
    addAndMakeVisible(osc1MixSlider);
    configureLabel(osc1MixLabel, "Mix");
    addAndMakeVisible(osc1MixLabel);
    osc1MixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc1mix", osc1MixSlider);
    osc1MixSlider.onValueChange = [this]() { updateFeedback("OSC 1 Mix", osc1MixSlider.getValue()); };

    // ========== OSCILLATOR 2 ==========
    configureRotary(osc2WaveSlider);
    osc2WaveSlider.setDoubleClickReturnValue(true, 0.5); // Default: Sawtooth
    addAndMakeVisible(osc2WaveSlider);
    configureLabel(osc2WaveLabel, "Wave");
    addAndMakeVisible(osc2WaveLabel);
    osc2WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc2wave", osc2WaveSlider);
    osc2WaveSlider.onValueChange = [this]() { updateFeedback("OSC 2 Wave", osc2WaveSlider.getValue()); };

    configureRotary(osc2CoarseSlider);
    osc2CoarseSlider.setDoubleClickReturnValue(true, 0); // Default: 0 semitones
    addAndMakeVisible(osc2CoarseSlider);
    configureLabel(osc2CoarseLabel, "Coarse");
    addAndMakeVisible(osc2CoarseLabel);
    osc2CoarseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc2coarse", osc2CoarseSlider);
    osc2CoarseSlider.onValueChange = [this]() { updateFeedback("OSC 2 Coarse", osc2CoarseSlider.getValue(), " semitones"); };

    configureRotary(osc2FineSlider);
    osc2FineSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 cents
    addAndMakeVisible(osc2FineSlider);
    configureLabel(osc2FineLabel, "Fine");
    addAndMakeVisible(osc2FineLabel);
    osc2FineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc2fine", osc2FineSlider);
    osc2FineSlider.onValueChange = [this]() { updateFeedback("OSC 2 Fine", osc2FineSlider.getValue(), " cents"); };

    configureRotary(osc2MixSlider);
    osc2MixSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0.0 (off)
    addAndMakeVisible(osc2MixSlider);
    configureLabel(osc2MixLabel, "Mix");
    addAndMakeVisible(osc2MixLabel);
    osc2MixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc2mix", osc2MixSlider);
    osc2MixSlider.onValueChange = [this]() { updateFeedback("OSC 2 Mix", osc2MixSlider.getValue()); };

    // ========== OSCILLATOR 3 ==========
    configureRotary(osc3WaveSlider);
    osc3WaveSlider.setDoubleClickReturnValue(true, 0.0); // Default: Sine
    addAndMakeVisible(osc3WaveSlider);
    configureLabel(osc3WaveLabel, "Wave");
    addAndMakeVisible(osc3WaveLabel);
    osc3WaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc3wave", osc3WaveSlider);
    osc3WaveSlider.onValueChange = [this]() { updateFeedback("OSC 3 Wave", osc3WaveSlider.getValue()); };

    configureRotary(osc3CoarseSlider);
    osc3CoarseSlider.setDoubleClickReturnValue(true, -12); // Default: -12 semitones (one octave down)
    addAndMakeVisible(osc3CoarseSlider);
    configureLabel(osc3CoarseLabel, "Coarse");
    addAndMakeVisible(osc3CoarseLabel);
    osc3CoarseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc3coarse", osc3CoarseSlider);
    osc3CoarseSlider.onValueChange = [this]() { updateFeedback("OSC 3 Coarse", osc3CoarseSlider.getValue(), " semitones"); };

    configureRotary(osc3FineSlider);
    osc3FineSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 cents
    addAndMakeVisible(osc3FineSlider);
    configureLabel(osc3FineLabel, "Fine");
    addAndMakeVisible(osc3FineLabel);
    osc3FineAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc3fine", osc3FineSlider);
    osc3FineSlider.onValueChange = [this]() { updateFeedback("OSC 3 Fine", osc3FineSlider.getValue(), " cents"); };

    configureRotary(osc3MixSlider);
    osc3MixSlider.setDoubleClickReturnValue(true, 0.5); // Default: 0.5
    addAndMakeVisible(osc3MixSlider);
    configureLabel(osc3MixLabel, "Mix");
    addAndMakeVisible(osc3MixLabel);
    osc3MixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "osc3mix", osc3MixSlider);
    osc3MixSlider.onValueChange = [this]() { updateFeedback("OSC 3 Mix", osc3MixSlider.getValue()); };

    // ========== GLOBAL CONTROLS ==========
    configureRotary(volumeSlider);
    volumeSlider.setDoubleClickReturnValue(true, 0.7); // Default: 0.7
    addAndMakeVisible(volumeSlider);
    configureLabel(volumeLabel, "Volume");
    addAndMakeVisible(volumeLabel);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "volume", volumeSlider);
    volumeSlider.onValueChange = [this]() { updateFeedback("Volume", volumeSlider.getValue()); };

    configureRotary(globalOctaveSlider);
    globalOctaveSlider.setDoubleClickReturnValue(true, 0); // Default: 0 octaves
    addAndMakeVisible(globalOctaveSlider);
    configureLabel(globalOctaveLabel, "Octave");
    addAndMakeVisible(globalOctaveLabel);
    globalOctaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "globaloctave", globalOctaveSlider);
    globalOctaveSlider.onValueChange = [this]() { updateFeedback("Global Octave", globalOctaveSlider.getValue(), " octaves"); };

    // ========== AMPLITUDE ADSR ==========
    configureRotary(ampAttackSlider);
    ampAttackSlider.setDoubleClickReturnValue(true, 0.003); // Default: 3ms
    addAndMakeVisible(ampAttackSlider);
    configureLabel(ampAttackLabel, "Attack");
    addAndMakeVisible(ampAttackLabel);
    ampAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampattack", ampAttackSlider);
    ampAttackSlider.onValueChange = [this]() { updateFeedback("Amp Attack", ampAttackSlider.getValue(), " s"); };

    configureRotary(ampDecaySlider);
    ampDecaySlider.setDoubleClickReturnValue(true, 0.3); // Default: 300ms
    addAndMakeVisible(ampDecaySlider);
    configureLabel(ampDecayLabel, "Decay");
    addAndMakeVisible(ampDecayLabel);
    ampDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampdecay", ampDecaySlider);
    ampDecaySlider.onValueChange = [this]() { updateFeedback("Amp Decay", ampDecaySlider.getValue(), " s"); };

    configureRotary(ampSustainSlider);
    ampSustainSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0.0
    addAndMakeVisible(ampSustainSlider);
    configureLabel(ampSustainLabel, "Sustain");
    addAndMakeVisible(ampSustainLabel);
    ampSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampsustain", ampSustainSlider);
    ampSustainSlider.onValueChange = [this]() { updateFeedback("Amp Sustain", ampSustainSlider.getValue()); };

    configureRotary(ampReleaseSlider);
    ampReleaseSlider.setDoubleClickReturnValue(true, 0.1); // Default: 100ms
    addAndMakeVisible(ampReleaseSlider);
    configureLabel(ampReleaseLabel, "Release");
    addAndMakeVisible(ampReleaseLabel);
    ampReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "amprelease", ampReleaseSlider);
    ampReleaseSlider.onValueChange = [this]() { updateFeedback("Amp Release", ampReleaseSlider.getValue(), " s"); };

    // ========== ANALOG CHARACTER ==========
    configureRotary(driftSlider);
    driftSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 (off)
    addAndMakeVisible(driftSlider);
    configureLabel(driftLabel, "Drift");
    addAndMakeVisible(driftLabel);
    driftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drift", driftSlider);
    driftSlider.onValueChange = [this]() { updateFeedback("Drift", driftSlider.getValue()); };

    configureRotary(phaseRandomSlider);
    phaseRandomSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 (off)
    addAndMakeVisible(phaseRandomSlider);
    configureLabel(phaseRandomLabel, "Phase Rnd");
    addAndMakeVisible(phaseRandomLabel);
    phaseRandomAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "phaserandom", phaseRandomSlider);
    phaseRandomSlider.onValueChange = [this]() { updateFeedback("Phase Random", phaseRandomSlider.getValue()); };

    configureRotary(unisonSlider);
    unisonSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 (off)
    addAndMakeVisible(unisonSlider);
    configureLabel(unisonLabel, "Unison");
    addAndMakeVisible(unisonLabel);
    unisonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "unison", unisonSlider);
    unisonSlider.onValueChange = [this]() { updateFeedback("Unison", unisonSlider.getValue()); };

    // ========== NOISE OSCILLATOR (Order: Type, Decay, Volume) ==========
    configureRotary(noiseTypeSlider);
    noiseTypeSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 (white noise)
    configureLabel(noiseTypeLabel, "Type");
    noiseTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noisetype", noiseTypeSlider);
    noiseTypeSlider.onValueChange = [this]() { updateFeedback("Noise Type", noiseTypeSlider.getValue()); };

    configureRotary(noiseDecaySlider);
    noiseDecaySlider.setDoubleClickReturnValue(true, 2.0); // Default: 2.0 (right = sustained)
    configureLabel(noiseDecayLabel, "Decay");
    noiseDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noisedecay", noiseDecaySlider);
    noiseDecaySlider.onValueChange = [this]() { updateFeedback("Noise Decay", noiseDecaySlider.getValue(), "s"); };

    configureRotary(noiseMixSlider);
    noiseMixSlider.setDoubleClickReturnValue(true, 0.0); // Default: 0 (off)
    configureLabel(noiseMixLabel, "Volume");
    noiseMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "noisemix", noiseMixSlider);
    noiseMixSlider.onValueChange = [this]() { updateFeedback("Noise", noiseMixSlider.getValue()); };

    // ========== CREATE OSCILLATOR TABS ==========
    // Create panel components for each tab
    auto* osc1Panel = new juce::Component();
    osc1Panel->addAndMakeVisible(osc1WaveSlider);
    osc1Panel->addAndMakeVisible(osc1WaveLabel);
    osc1Panel->addAndMakeVisible(osc1CoarseSlider);
    osc1Panel->addAndMakeVisible(osc1CoarseLabel);
    osc1Panel->addAndMakeVisible(osc1FineSlider);
    osc1Panel->addAndMakeVisible(osc1FineLabel);
    osc1Panel->addAndMakeVisible(osc1MixSlider);
    osc1Panel->addAndMakeVisible(osc1MixLabel);

    auto* osc2Panel = new juce::Component();
    osc2Panel->addAndMakeVisible(osc2WaveSlider);
    osc2Panel->addAndMakeVisible(osc2WaveLabel);
    osc2Panel->addAndMakeVisible(osc2CoarseSlider);
    osc2Panel->addAndMakeVisible(osc2CoarseLabel);
    osc2Panel->addAndMakeVisible(osc2FineSlider);
    osc2Panel->addAndMakeVisible(osc2FineLabel);
    osc2Panel->addAndMakeVisible(osc2MixSlider);
    osc2Panel->addAndMakeVisible(osc2MixLabel);

    auto* osc3Panel = new juce::Component();
    osc3Panel->addAndMakeVisible(osc3WaveSlider);
    osc3Panel->addAndMakeVisible(osc3WaveLabel);
    osc3Panel->addAndMakeVisible(osc3CoarseSlider);
    osc3Panel->addAndMakeVisible(osc3CoarseLabel);
    osc3Panel->addAndMakeVisible(osc3FineSlider);
    osc3Panel->addAndMakeVisible(osc3FineLabel);
    osc3Panel->addAndMakeVisible(osc3MixSlider);
    osc3Panel->addAndMakeVisible(osc3MixLabel);

    auto* noisePanel = new juce::Component();
    noisePanel->addAndMakeVisible(noiseTypeSlider);
    noisePanel->addAndMakeVisible(noiseTypeLabel);
    noisePanel->addAndMakeVisible(noiseDecaySlider);
    noisePanel->addAndMakeVisible(noiseDecayLabel);
    noisePanel->addAndMakeVisible(noiseMixSlider);
    noisePanel->addAndMakeVisible(noiseMixLabel);

    // Add tabs
    oscTabs.addTab("Osc 1", juce::Colour(0xff3a3a3a), osc1Panel, true);
    oscTabs.addTab("Osc 2", juce::Colour(0xff3a3a3a), osc2Panel, true);
    oscTabs.addTab("Osc 3", juce::Colour(0xff3a3a3a), osc3Panel, true);
    oscTabs.addTab("Noise", juce::Colour(0xff3a3a3a), noisePanel, true);

    // Remove default JUCE tab outline/border
    oscTabs.setOutline(0);
    oscTabs.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff3a3a3a));
    oscTabs.setIndent(0); // Remove content area indent for alignment

    addAndMakeVisible(oscTabs);
}

OscTab::~OscTab()
{
}

void OscTab::updateFeedback(const juce::String& paramName, float value, const juce::String& unit)
{
    juce::String displayText = paramName + ": " + juce::String(value, 2) + unit;
    editor.showMessage(displayText);
}

//==============================================================================
void OscTab::paint(juce::Graphics& g)
{
    // Draw panel background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Calculate layout positions (must match resized())
    const int columnSpacing = 130;
    const int oscSectionWidth = 20 + (columnSpacing * 4);
    const int analogBoxWidth = 150; // Narrower analog section
    const int analogBoxMargin = 20;

    const int tabsX = 20;
    const int tabsY = 60;
    const int tabsHeight = 130;

    const int box2Y = tabsY + tabsHeight + 10;
    const int box2Height = 120; // Increased for more bottom padding

    const int box3Y = box2Y + box2Height + 10;
    const int box3Height = 120; // Increased for more bottom padding

    const int analogBoxX = tabsX + oscSectionWidth + analogBoxMargin;
    const int analogBoxY = tabsY;
    const int analogBoxHeight = (box3Y + box3Height) - tabsY;

    // Draw section backgrounds
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(tabsX, tabsY, oscSectionWidth, tabsHeight, 10); // Oscillator tabs
    g.fillRoundedRectangle(tabsX, box2Y, oscSectionWidth, box2Height, 10); // Amp ADSR
    g.fillRoundedRectangle(tabsX, box3Y, oscSectionWidth, box3Height, 10); // Global controls
    g.fillRoundedRectangle(analogBoxX, analogBoxY, analogBoxWidth, analogBoxHeight, 10); // Analog (full height)

    // Draw section labels
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);
    g.drawText("AMP ENVELOPE", tabsX + 10, box2Y + 5, 200, 20, juce::Justification::left);
    g.drawText("GLOBAL CONTROLS", tabsX + 10, box3Y + 5, 200, 20, juce::Justification::left);
    g.drawText("ANALOG", analogBoxX + 10, analogBoxY + 5, 160, 20, juce::Justification::left);
}

void OscTab::resized()
{
    const int knobSize = 60;
    const int labelHeight = 20;
    const int columnSpacing = 130;

    // Preset controls at the top (Label -> Selector -> Save button)
    presetLabel.setBounds(getWidth() - 330, 15, 100, 20);
    presetSelector.setBounds(getWidth() - 220, 15, 130, 25);
    savePresetButton.setBounds(getWidth() - 85, 15, 50, 25);

    // Calculate widths: 4 dials worth of space
    const int oscSectionWidth = 20 + (columnSpacing * 4); // Start margin + 4 columns = 540px
    const int analogBoxWidth = 150; // Narrower analog section
    const int analogBoxMargin = 20;

    // OSCILLATOR TABS (left side) - compact size, only as wide as needed
    const int tabsX = 20;
    const int tabsY = 60;
    const int tabsWidth = oscSectionWidth;
    const int tabsHeight = 130; // Compact: just enough for 4 dials + labels
    oscTabs.setBounds(tabsX, tabsY, tabsWidth, tabsHeight);

    // Layout controls within each tab panel
    auto* osc1Panel = oscTabs.getTabContentComponent(0);
    auto* osc2Panel = oscTabs.getTabContentComponent(1);
    auto* osc3Panel = oscTabs.getTabContentComponent(2);
    auto* noisePanel = oscTabs.getTabContentComponent(3);

    const int tabPanelY = 10; // Inside the tab - closer to top
    const int tabStartX = 0; // Align with dials below (no extra left margin)

    // Osc 1 panel layout
    if (osc1Panel)
    {
        osc1WaveSlider.setBounds(tabStartX, tabPanelY, knobSize, knobSize);
        osc1WaveLabel.setBounds(tabStartX, tabPanelY + knobSize, knobSize, labelHeight);

        osc1CoarseSlider.setBounds(tabStartX + columnSpacing, tabPanelY, knobSize, knobSize);
        osc1CoarseLabel.setBounds(tabStartX + columnSpacing, tabPanelY + knobSize, knobSize, labelHeight);

        osc1FineSlider.setBounds(tabStartX + columnSpacing * 2, tabPanelY, knobSize, knobSize);
        osc1FineLabel.setBounds(tabStartX + columnSpacing * 2, tabPanelY + knobSize, knobSize, labelHeight);

        osc1MixSlider.setBounds(tabStartX + columnSpacing * 3, tabPanelY, knobSize, knobSize);
        osc1MixLabel.setBounds(tabStartX + columnSpacing * 3, tabPanelY + knobSize, knobSize, labelHeight);
    }

    // Osc 2 panel layout
    if (osc2Panel)
    {
        osc2WaveSlider.setBounds(tabStartX, tabPanelY, knobSize, knobSize);
        osc2WaveLabel.setBounds(tabStartX, tabPanelY + knobSize, knobSize, labelHeight);

        osc2CoarseSlider.setBounds(tabStartX + columnSpacing, tabPanelY, knobSize, knobSize);
        osc2CoarseLabel.setBounds(tabStartX + columnSpacing, tabPanelY + knobSize, knobSize, labelHeight);

        osc2FineSlider.setBounds(tabStartX + columnSpacing * 2, tabPanelY, knobSize, knobSize);
        osc2FineLabel.setBounds(tabStartX + columnSpacing * 2, tabPanelY + knobSize, knobSize, labelHeight);

        osc2MixSlider.setBounds(tabStartX + columnSpacing * 3, tabPanelY, knobSize, knobSize);
        osc2MixLabel.setBounds(tabStartX + columnSpacing * 3, tabPanelY + knobSize, knobSize, labelHeight);
    }

    // Osc 3 panel layout
    if (osc3Panel)
    {
        osc3WaveSlider.setBounds(tabStartX, tabPanelY, knobSize, knobSize);
        osc3WaveLabel.setBounds(tabStartX, tabPanelY + knobSize, knobSize, labelHeight);

        osc3CoarseSlider.setBounds(tabStartX + columnSpacing, tabPanelY, knobSize, knobSize);
        osc3CoarseLabel.setBounds(tabStartX + columnSpacing, tabPanelY + knobSize, knobSize, labelHeight);

        osc3FineSlider.setBounds(tabStartX + columnSpacing * 2, tabPanelY, knobSize, knobSize);
        osc3FineLabel.setBounds(tabStartX + columnSpacing * 2, tabPanelY + knobSize, knobSize, labelHeight);

        osc3MixSlider.setBounds(tabStartX + columnSpacing * 3, tabPanelY, knobSize, knobSize);
        osc3MixLabel.setBounds(tabStartX + columnSpacing * 3, tabPanelY + knobSize, knobSize, labelHeight);
    }

    // Noise panel layout (order: Type, Decay, Volume)
    if (noisePanel)
    {
        noiseTypeSlider.setBounds(tabStartX, tabPanelY, knobSize, knobSize);
        noiseTypeLabel.setBounds(tabStartX, tabPanelY + knobSize, knobSize, labelHeight);

        noiseDecaySlider.setBounds(tabStartX + columnSpacing, tabPanelY, knobSize, knobSize);
        noiseDecayLabel.setBounds(tabStartX + columnSpacing, tabPanelY + knobSize, knobSize, labelHeight);

        noiseMixSlider.setBounds(tabStartX + columnSpacing * 2, tabPanelY, knobSize, knobSize);
        noiseMixLabel.setBounds(tabStartX + columnSpacing * 2, tabPanelY + knobSize, knobSize, labelHeight);
    }

    // BOX 2: AMP ADSR (aligned with osc section)
    const int box2Y = tabsY + tabsHeight + 10; // Right below tabs
    const int box2Height = 120; // Increased for more bottom padding
    const int ampStartX = tabsX;
    const int box2Row1Y = box2Y + 30; // Position inside box

    ampAttackSlider.setBounds(ampStartX, box2Row1Y, knobSize, knobSize);
    ampAttackLabel.setBounds(ampStartX, box2Row1Y + knobSize, knobSize, labelHeight);

    ampDecaySlider.setBounds(ampStartX + columnSpacing, box2Row1Y, knobSize, knobSize);
    ampDecayLabel.setBounds(ampStartX + columnSpacing, box2Row1Y + knobSize, knobSize, labelHeight);

    ampSustainSlider.setBounds(ampStartX + columnSpacing * 2, box2Row1Y, knobSize, knobSize);
    ampSustainLabel.setBounds(ampStartX + columnSpacing * 2, box2Row1Y + knobSize, knobSize, labelHeight);

    ampReleaseSlider.setBounds(ampStartX + columnSpacing * 3, box2Row1Y, knobSize, knobSize);
    ampReleaseLabel.setBounds(ampStartX + columnSpacing * 3, box2Row1Y + knobSize, knobSize, labelHeight);

    // BOX 3: GLOBAL CONTROLS (aligned with osc section)
    const int box3Y = box2Y + box2Height + 10; // Right below amp ADSR
    const int box3Height = 120; // Increased for more bottom padding
    const int box3Row1Y = box3Y + 30; // Position inside box

    globalOctaveSlider.setBounds(ampStartX, box3Row1Y, knobSize, knobSize);
    globalOctaveLabel.setBounds(ampStartX, box3Row1Y + knobSize, knobSize, labelHeight);

    volumeSlider.setBounds(ampStartX + columnSpacing * 3, box3Row1Y, knobSize, knobSize);
    volumeLabel.setBounds(ampStartX + columnSpacing * 3, box3Row1Y + knobSize, knobSize, labelHeight);

    // ANALOG CHARACTER CONTROLS (right side, spans full height)
    const int analogBoxX = tabsX + oscSectionWidth + analogBoxMargin;
    const int analogBoxY = tabsY;
    const int analogBoxHeight = (box3Y + box3Height) - tabsY; // Full height from osc to amp

    const int analogCenterX = analogBoxX + (analogBoxWidth - knobSize) / 2; // Center horizontally

    // Calculate exact Y position of dials in Osc tabs
    // TabbedComponent adds a tab bar, then the content area starts
    // The actual tab bar in JUCE TabbedComponent is typically 24-28px depending on look and feel
    const int tabBarHeight = oscTabs.getTabBarDepth(); // Get actual tab bar height
    const int oscDialsY = tabsY + tabBarHeight + tabPanelY;

    // Align Drift with Osc tab dials
    driftSlider.setBounds(analogCenterX, oscDialsY, knobSize, knobSize);
    driftLabel.setBounds(analogCenterX, oscDialsY + knobSize, knobSize, labelHeight);

    // Align Phase Random with Amp ADSR dials (box2Row1Y)
    phaseRandomSlider.setBounds(analogCenterX, box2Row1Y, knobSize, knobSize);
    phaseRandomLabel.setBounds(analogCenterX, box2Row1Y + knobSize, knobSize, labelHeight);

    // Align Unison with Global controls dials (box3Row1Y)
    unisonSlider.setBounds(analogCenterX, box3Row1Y, knobSize, knobSize);
    unisonLabel.setBounds(analogCenterX, box3Row1Y + knobSize, knobSize, labelHeight);
}
