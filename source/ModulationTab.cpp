#include "PluginProcessor.h"
#include "ModulationTab.h"

//==============================================================================
ModulationTab::ModulationTab(AcidSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Setup labels for each row
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(cutoffLabel);

    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(resonanceLabel);

    envModLabel.setText("Env Mod", juce::dontSendNotification);
    envModLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(envModLabel);

    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(decayLabel);

    accentLabel.setText("Accent", juce::dontSendNotification);
    accentLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(accentLabel);

    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(waveformLabel);

    subOscLabel.setText("Sub Osc", juce::dontSendNotification);
    subOscLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subOscLabel);

    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(driveLabel);

    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);

    delayMixLabel.setText("Delay Mix", juce::dontSendNotification);
    delayMixLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(delayMixLabel);

    // Setup all 10 dedicated LFO control sets
    setupLFOControls(cutoffLFO, "cutofflfor", "cutofflfow", "cutofflfod");
    setupLFOControls(resonanceLFO, "resonancelforate", "resonancelfowave", "resonancelfodepth");
    setupLFOControls(envModLFO, "envmodlforate", "envmodlfowave", "envmodlfodepth");
    setupLFOControls(decayLFO, "decaylforate", "decaylfowave", "decaylfodepth");
    setupLFOControls(accentLFO, "accentlforate", "accentlfowave", "accentlfodepth");
    setupLFOControls(waveformLFO, "waveformlforate", "waveformlfowave", "waveformlfodepth");
    setupLFOControls(subOscLFO, "subosclforate", "subosclfowave", "subosclfodepth");
    setupLFOControls(driveLFO, "drivelforate", "drivelfowave", "drivelfodepth");
    setupLFOControls(volumeLFO, "volumelforate", "volumelfowave", "volumelfodepth");
    setupLFOControls(delayMixLFO, "delaymixlforate", "delaymixlfowave", "delaymixlfodepth");
}

ModulationTab::~ModulationTab()
{
}

void ModulationTab::setupLFOControls(LFOControls& lfo,
                                 const juce::String& rateParamID,
                                 const juce::String& waveParamID,
                                 const juce::String& depthParamID)
{
    // Setup LFO Rate selector (IDs must start at 1, not 0 - JUCE requirement)
    lfo.rateSelector.addItem("1/16", 1);
    lfo.rateSelector.addItem("1/8", 2);
    lfo.rateSelector.addItem("1/4", 3);
    lfo.rateSelector.addItem("1/3", 4);
    lfo.rateSelector.addItem("1/2", 5);
    lfo.rateSelector.addItem("3/4", 6);
    lfo.rateSelector.addItem("1/1", 7);
    lfo.rateSelector.addItem("3/2", 8);
    lfo.rateSelector.addItem("2/1", 9);
    lfo.rateSelector.addItem("3/1", 10);
    lfo.rateSelector.addItem("4/1", 11);
    lfo.rateSelector.addItem("6/1", 12);
    lfo.rateSelector.addItem("8/1", 13);
    lfo.rateSelector.addItem("12/1", 14);
    lfo.rateSelector.addItem("16/1", 15);
    lfo.rateSelector.setTooltip("Rate");
    addAndMakeVisible(lfo.rateSelector);
    lfo.rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), rateParamID, lfo.rateSelector);

    // Setup LFO Waveform selector (IDs must start at 1, not 0 - JUCE requirement)
    lfo.waveformSelector.addItem("Sine", 1);
    lfo.waveformSelector.addItem("Triangle", 2);
    lfo.waveformSelector.addItem("Saw Up", 3);
    lfo.waveformSelector.addItem("Saw Dn", 4);
    lfo.waveformSelector.addItem("Square", 5);
    lfo.waveformSelector.addItem("Random", 6);
    lfo.waveformSelector.setTooltip("Wave");
    addAndMakeVisible(lfo.waveformSelector);
    lfo.waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), waveParamID, lfo.waveformSelector);

    // Setup LFO Depth slider
    lfo.depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    lfo.depthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    lfo.depthSlider.setTooltip("Depth");
    addAndMakeVisible(lfo.depthSlider);
    lfo.depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), depthParamID, lfo.depthSlider);
}

//==============================================================================
void ModulationTab::paint(juce::Graphics& g)
{
    // Draw panel background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw row separators
    g.setColour(juce::Colour(0xff3a3a3a));
    for (int i = 1; i < 10; ++i)
    {
        int y = 10 + i * 55;
        g.drawLine(10.0f, static_cast<float>(y), static_cast<float>(getWidth() - 10), static_cast<float>(y), 1.0f);
    }

    // Draw column headers
    g.setColour(juce::Colours::lightgrey);
    g.setFont(12.0f);
    g.drawText("Parameter", 20, 0, 100, 15, juce::Justification::left);
    g.drawText("Rate", 200, 0, 80, 15, juce::Justification::centred);
    g.drawText("Wave", 320, 0, 80, 15, juce::Justification::centred);
    g.drawText("Depth", 440, 0, 50, 15, juce::Justification::centred);
}

void ModulationTab::resized()
{
    const int knobSize = 45;
    const int rowHeight = 55;
    const int startY = 25;
    const int paramLabelX = 20;
    const int lfoStartX = 200;
    const int lfoSpacing = 110;

    // Row-based layout helper lambda
    auto layoutRow = [&](int rowIndex, juce::Label& paramLabel, LFOControls& lfo)
    {
        int y = startY + rowIndex * rowHeight;

        // Parameter label
        paramLabel.setBounds(paramLabelX, y + 15, 150, 20);

        // LFO Rate selector
        lfo.rateSelector.setBounds(lfoStartX, y + 12, 80, 26);

        // LFO Waveform selector
        lfo.waveformSelector.setBounds(lfoStartX + lfoSpacing, y + 12, 80, 26);

        // LFO Depth knob
        lfo.depthSlider.setBounds(lfoStartX + lfoSpacing * 2, y, knobSize, knobSize);
    };

    // Layout all 10 rows with LFO controls
    layoutRow(0, cutoffLabel, cutoffLFO);
    layoutRow(1, resonanceLabel, resonanceLFO);
    layoutRow(2, envModLabel, envModLFO);
    layoutRow(3, decayLabel, decayLFO);
    layoutRow(4, accentLabel, accentLFO);
    layoutRow(5, waveformLabel, waveformLFO);
    layoutRow(6, subOscLabel, subOscLFO);
    layoutRow(7, driveLabel, driveLFO);
    layoutRow(8, volumeLabel, volumeLFO);
    layoutRow(9, delayMixLabel, delayMixLFO);
}
