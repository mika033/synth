#include "PluginProcessor.h"
#include "SequencerTab.h"

//==============================================================================
SequencerTab::SequencerTab(AcidSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Title label
    titleLabel.setText("ARPEGGIATOR", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // Arpeggiator On/Off toggle
    arpOnOffToggle.setButtonText("Enable");
    addAndMakeVisible(arpOnOffToggle);
    arpOnOffLabel.setText("Arpeggiator", juce::dontSendNotification);
    arpOnOffLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(arpOnOffLabel);
    arpOnOffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.getValueTreeState(), "arponoff", arpOnOffToggle);

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
    arpRateSelector.addItem("1/16", 2);
    arpRateSelector.addItem("1/16T", 3);
    arpRateSelector.addItem("1/8", 4);
    arpRateSelector.addItem("1/8T", 5);
    arpRateSelector.addItem("1/4", 6);
    arpRateSelector.addItem("1/4T", 7);
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

    // Arpeggiator Swing slider
    arpSwingSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    arpSwingSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(arpSwingSlider);
    arpSwingLabel.setText("Swing", juce::dontSendNotification);
    arpSwingLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(arpSwingLabel);
    arpSwingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "arpswing", arpSwingSlider);
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
    const int knobSize = 80;
    const int labelHeight = 20;
    const int controlY = 150;
    const int spacing = 150;
    const int startX = 80;

    // Title
    titleLabel.setBounds(0, 20, getWidth(), 30);

    // Row 1: On/Off toggle
    arpOnOffLabel.setBounds(startX, 80, 120, labelHeight);
    arpOnOffToggle.setBounds(startX + 120, 80, 80, 30);

    // Row 2: Mode selector
    arpModeLabel.setBounds(startX, 120, 120, labelHeight);
    arpModeSelector.setBounds(startX + 120, 120, 120, 25);

    // Row 3: Rate selector
    arpRateLabel.setBounds(startX, 160, 120, labelHeight);
    arpRateSelector.setBounds(startX + 120, 160, 120, 25);

    // Row 4: Octaves, Gate, Octave Shift, and Swing knobs
    int knobsY = 220;
    arpOctavesLabel.setBounds(startX, knobsY - labelHeight, knobSize, labelHeight);
    arpOctavesSlider.setBounds(startX, knobsY, knobSize, knobSize);

    arpGateLabel.setBounds(startX + spacing, knobsY - labelHeight, knobSize + 20, labelHeight);
    arpGateSlider.setBounds(startX + spacing, knobsY, knobSize, knobSize);

    arpOctaveShiftLabel.setBounds(startX + spacing * 2, knobsY - labelHeight, knobSize + 20, labelHeight);
    arpOctaveShiftSlider.setBounds(startX + spacing * 2, knobsY, knobSize, knobSize);

    arpSwingLabel.setBounds(startX + spacing * 3, knobsY - labelHeight, knobSize, labelHeight);
    arpSwingSlider.setBounds(startX + spacing * 3, knobsY, knobSize, knobSize);
}
