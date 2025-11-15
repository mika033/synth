#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AcidSynthAudioProcessorEditor::AcidSynthAudioProcessorEditor(AcidSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Set editor size (bigger to fit all controls)
    setSize(750, 520);

    // Configure cutoff slider
    cutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(cutoffSlider);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffLabel);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "cutoff", cutoffSlider);

    // Configure resonance slider
    resonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(resonanceSlider);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceLabel);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "resonance", resonanceSlider);

    // Configure envelope mod slider
    envModSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    envModSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(envModSlider);
    envModLabel.setText("Env Mod", juce::dontSendNotification);
    envModLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(envModLabel);
    envModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "envmod", envModSlider);

    // Configure decay slider
    decaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(decaySlider);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(decayLabel);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "decay", decaySlider);

    // Configure accent slider
    accentSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    accentSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(accentSlider);
    accentLabel.setText("Accent", juce::dontSendNotification);
    accentLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentLabel);
    accentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "accent", accentSlider);

    // Configure sub-oscillator slider
    subOscSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    subOscSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(subOscSlider);
    subOscLabel.setText("Sub Osc", juce::dontSendNotification);
    subOscLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subOscLabel);
    subOscAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "subosc", subOscSlider);

    // Configure drive slider
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drive", driveSlider);

    // Configure volume slider
    volumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(volumeSlider);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "volume", volumeSlider);

    // Configure waveform selector
    waveformSelector.addItem("Saw", 1);
    waveformSelector.addItem("Square", 2);
    addAndMakeVisible(waveformSelector);
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(waveformLabel);
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "waveform", waveformSelector);

    // Configure preset selector
    presetSelector.addItem("Classic 303 Bass", 1);
    presetSelector.addItem("Squelchy Lead", 2);
    presetSelector.addItem("Deep Rumble", 3);
    presetSelector.addItem("Aggressive Lead", 4);
    presetSelector.addItem("Init", 5);
    presetSelector.onChange = [this]
    {
        int selectedIndex = presetSelector.getSelectedItemIndex();
        if (selectedIndex >= 0)
            audioProcessor.setCurrentProgram(selectedIndex);
    };
    addAndMakeVisible(presetSelector);
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetLabel);

    // Configure LFO Rate selector
    lfoRateSelector.addItem("1/16", 1);
    lfoRateSelector.addItem("1/8", 2);
    lfoRateSelector.addItem("1/4", 3);
    lfoRateSelector.addItem("1/2", 4);
    lfoRateSelector.addItem("1/1", 5);
    addAndMakeVisible(lfoRateSelector);
    lfoRateLabel.setText("LFO Rate", juce::dontSendNotification);
    lfoRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoRateLabel);
    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "lforate", lfoRateSelector);

    // Configure LFO Destination selector
    lfoDestSelector.addItem("Off", 1);
    lfoDestSelector.addItem("Cutoff", 2);
    lfoDestSelector.addItem("Resonance", 3);
    lfoDestSelector.addItem("Volume", 4);
    addAndMakeVisible(lfoDestSelector);
    lfoDestLabel.setText("LFO Dest", juce::dontSendNotification);
    lfoDestLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoDestLabel);
    lfoDestAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "lfodest", lfoDestSelector);

    // Configure LFO Depth slider
    lfoDepthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    lfoDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(lfoDepthSlider);
    lfoDepthLabel.setText("LFO Depth", juce::dontSendNotification);
    lfoDepthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(lfoDepthLabel);
    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "lfodepth", lfoDepthSlider);

    // Configure Delay Time selector
    delayTimeSelector.addItem("1/16", 1);
    delayTimeSelector.addItem("1/8", 2);
    delayTimeSelector.addItem("1/4", 3);
    delayTimeSelector.addItem("1/2", 4);
    delayTimeSelector.addItem("1/1", 5);
    addAndMakeVisible(delayTimeSelector);
    delayTimeLabel.setText("Delay Time", juce::dontSendNotification);
    delayTimeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayTimeLabel);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "delaytime", delayTimeSelector);

    // Configure Delay Feedback slider
    delayFeedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayFeedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackLabel.setText("Delay FB", juce::dontSendNotification);
    delayFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayFeedbackLabel);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "delayfeedback", delayFeedbackSlider);

    // Configure Delay Mix slider
    delayMixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayMixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(delayMixSlider);
    delayMixLabel.setText("Delay Mix", juce::dontSendNotification);
    delayMixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayMixLabel);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "delaymix", delayMixSlider);
}

AcidSynthAudioProcessorEditor::~AcidSynthAudioProcessorEditor()
{
}

//==============================================================================
void AcidSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(24.0f, juce::Font::bold));
    g.drawText("ACID SYNTH", 0, 10, getWidth(), 40, juce::Justification::centred);

    // Draw panel background
    g.setColour(juce::Colour(0xff2a2a2a));
    g.fillRoundedRectangle(20, 60, getWidth() - 40, getHeight() - 80, 10);
}

void AcidSynthAudioProcessorEditor::resized()
{
    const int knobSize = 70;
    const int knobSpacing = 85;
    const int startX = 30;
    const int startY = 80;

    // Preset selector at the top right
    presetLabel.setBounds(getWidth() - 220, 15, 60, 20);
    presetSelector.setBounds(getWidth() - 150, 15, 120, 25);

    // First row - Filter controls
    cutoffLabel.setBounds(startX, startY - 20, knobSize, 20);
    cutoffSlider.setBounds(startX, startY, knobSize, knobSize);

    resonanceLabel.setBounds(startX + knobSpacing, startY - 20, knobSize, 20);
    resonanceSlider.setBounds(startX + knobSpacing, startY, knobSize, knobSize);

    envModLabel.setBounds(startX + knobSpacing * 2, startY - 20, knobSize, 20);
    envModSlider.setBounds(startX + knobSpacing * 2, startY, knobSize, knobSize);

    decayLabel.setBounds(startX + knobSpacing * 3, startY - 20, knobSize, 20);
    decaySlider.setBounds(startX + knobSpacing * 3, startY, knobSize, knobSize);

    accentLabel.setBounds(startX + knobSpacing * 4, startY - 20, knobSize, 20);
    accentSlider.setBounds(startX + knobSpacing * 4, startY, knobSize, knobSize);

    // Second row - Tone shaping controls
    const int row2Y = startY + 120;

    subOscLabel.setBounds(startX, row2Y - 20, knobSize, 20);
    subOscSlider.setBounds(startX, row2Y, knobSize, knobSize);

    driveLabel.setBounds(startX + knobSpacing, row2Y - 20, knobSize, 20);
    driveSlider.setBounds(startX + knobSpacing, row2Y, knobSize, knobSize);

    volumeLabel.setBounds(startX + knobSpacing * 2, row2Y - 20, knobSize, 20);
    volumeSlider.setBounds(startX + knobSpacing * 2, row2Y, knobSize, knobSize);

    // Waveform selector
    waveformLabel.setBounds(startX + knobSpacing * 3, row2Y - 20, knobSize, 20);
    waveformSelector.setBounds(startX + knobSpacing * 3, row2Y + 20, knobSize + 30, 30);

    // Third row - LFO controls
    const int row3Y = startY + 240;

    lfoRateLabel.setBounds(startX, row3Y - 20, knobSize, 20);
    lfoRateSelector.setBounds(startX, row3Y + 20, knobSize + 30, 30);

    lfoDestLabel.setBounds(startX + knobSpacing, row3Y - 20, knobSize, 20);
    lfoDestSelector.setBounds(startX + knobSpacing, row3Y + 20, knobSize + 30, 30);

    lfoDepthLabel.setBounds(startX + knobSpacing * 2, row3Y - 20, knobSize, 20);
    lfoDepthSlider.setBounds(startX + knobSpacing * 2, row3Y, knobSize, knobSize);

    // Fourth row - Delay controls
    const int row4Y = startY + 360;

    delayTimeLabel.setBounds(startX, row4Y - 20, knobSize, 20);
    delayTimeSelector.setBounds(startX, row4Y + 20, knobSize + 30, 30);

    delayFeedbackLabel.setBounds(startX + knobSpacing, row4Y - 20, knobSize, 20);
    delayFeedbackSlider.setBounds(startX + knobSpacing, row4Y, knobSize, knobSize);

    delayMixLabel.setBounds(startX + knobSpacing * 2, row4Y - 20, knobSize, 20);
    delayMixSlider.setBounds(startX + knobSpacing * 2, row4Y, knobSize, knobSize);
}
