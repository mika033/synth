#include "PluginProcessor.h"
#include "SequencerTab.h"

//==============================================================================
SequencerTab::SequencerTab(SnorkelSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Arpeggiator On/Off toggle
    arpOnOffToggle.setButtonText("Enabled");
    addAndMakeVisible(arpOnOffToggle);
    arpOnOffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "arponoff", arpOnOffToggle);

    // When arp is enabled, disable sequencer
    arpOnOffToggle.onClick = [this]()
    {
        if (arpOnOffToggle.getToggleState())
        {
            // Disable sequencer when arp is enabled
            auto* seqParam = audioProcessor.getValueTreeState().getParameter("seqenabled");
            if (seqParam != nullptr)
                seqParam->setValueNotifyingHost(0.0f);
        }
    };

    // Arpeggiator Mode selector (IDs must start at 1, not 0 - JUCE requirement)
    arpModeSelector.addItem("Up", 1);
    arpModeSelector.addItem("Down", 2);
    arpModeSelector.addItem("Up-Down", 3);
    arpModeSelector.addItem("Random", 4);
    arpModeSelector.addItem("As Played", 5);
    addAndMakeVisible(arpModeSelector);
    arpModeLabel.setText("Mode", juce::dontSendNotification);
    arpModeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(arpModeLabel);
    arpModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "arpmode", arpModeSelector);

    // Arpeggiator Rate selector (IDs must start at 1, not 0 - JUCE requirement)
    arpRateSelector.addItem("1/32", 1);
    arpRateSelector.addItem("1/32.", 2);
    arpRateSelector.addItem("1/16", 3);
    arpRateSelector.addItem("1/16.", 4);
    arpRateSelector.addItem("1/16T", 5);
    arpRateSelector.addItem("1/8", 6);
    arpRateSelector.addItem("1/8.", 7);
    arpRateSelector.addItem("1/8T", 8);
    arpRateSelector.addItem("1/4", 9);
    arpRateSelector.addItem("1/4.", 10);
    arpRateSelector.addItem("1/4T", 11);
    arpRateSelector.addItem("1/2", 12);
    arpRateSelector.addItem("1/2.", 13);
    arpRateSelector.addItem("1/1", 14);
    addAndMakeVisible(arpRateSelector);
    arpRateLabel.setText("Rate", juce::dontSendNotification);
    arpRateLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(arpRateLabel);
    arpRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "arprate", arpRateSelector);

    // Arpeggiator Octaves slider
    arpOctavesSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    arpOctavesSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(arpOctavesSlider);
    arpOctavesLabel.setText("Octaves", juce::dontSendNotification);
    arpOctavesLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(arpOctavesLabel);
    arpOctavesAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "arpoctaves", arpOctavesSlider);

    // Arpeggiator Gate slider
    arpGateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    arpGateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(arpGateSlider);
    arpGateLabel.setText("Gate Length", juce::dontSendNotification);
    arpGateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(arpGateLabel);
    arpGateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "arpgate", arpGateSlider);

    // Arpeggiator Octave Shift slider
    arpOctaveShiftSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    arpOctaveShiftSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(arpOctaveShiftSlider);
    arpOctaveShiftLabel.setText("Octave Shift", juce::dontSendNotification);
    arpOctaveShiftLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(arpOctaveShiftLabel);
    arpOctaveShiftAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "arpoctaveshift", arpOctaveShiftSlider);
}

SequencerTab::~SequencerTab()
{
}

//==============================================================================
void SequencerTab::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw panel for arpeggiator controls
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(30, 70, getWidth() - 60, 350, 10);
}

void SequencerTab::resized()
{
    const int knobSize = 60;
    const int labelHeight = 20;
    const int controlY = 150;
    const int spacing = 120;
    const int startX = 80;

    // Row 1: On/Off toggle
    arpOnOffToggle.setBounds(startX, 20, 80, 30);

    // Row 2: Mode selector
    arpModeLabel.setBounds(startX, 80, 120, labelHeight);
    arpModeSelector.setBounds(startX + 120, 80, 120, 25);

    // Row 3: Rate selector
    arpRateLabel.setBounds(startX, 120, 120, labelHeight);
    arpRateSelector.setBounds(startX + 120, 120, 120, 25);

    // Row 4: Octaves, Gate, Octave Shift knobs
    int knobsY = 180;
    arpOctavesLabel.setBounds(startX, knobsY - labelHeight, knobSize, labelHeight);
    arpOctavesSlider.setBounds(startX, knobsY, knobSize, knobSize);

    arpGateLabel.setBounds(startX + spacing, knobsY - labelHeight, knobSize + 20, labelHeight);
    arpGateSlider.setBounds(startX + spacing, knobsY, knobSize, knobSize);

    arpOctaveShiftLabel.setBounds(startX + spacing * 2, knobsY - labelHeight, knobSize + 20, labelHeight);
    arpOctaveShiftSlider.setBounds(startX + spacing * 2, knobsY, knobSize, knobSize);
}
