#include "PluginProcessor.h"
#include "SynthTab.h"

//==============================================================================
SynthTab::SynthTab(AcidSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Configure preset selector (IDs must start at 1, not 0 - JUCE requirement)
    presetSelector.addItem("Classic 303 Bass", 1);
    presetSelector.addItem("Squelchy Lead", 2);
    presetSelector.addItem("Deep Rumble", 3);
    presetSelector.addItem("Aggressive Lead", 4);
    presetSelector.addItem("Pulsing Bass", 5);
    presetSelector.addItem("Dub Delay Bass", 6);
    presetSelector.addItem("Wobble Bass", 7);
    presetSelector.addItem("Soft Pad", 8);
    presetSelector.addItem("Smooth Lead", 9);
    presetSelector.addItem("Warm Bass", 10);
    presetSelector.addItem("Evolving Pad", 11);
    presetSelector.addItem("Init", 12);
    presetSelector.setSelectedId(12); // Select Init preset by default
    presetSelector.onChange = [this]
    {
        int selectedIndex = presetSelector.getSelectedItemIndex();
        if (selectedIndex >= 0)
            audioProcessor.setCurrentProgram(selectedIndex);
    };
    addAndMakeVisible(presetSelector);
    presetLabel.setText("Preset", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(presetLabel);

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

    // Old decay slider removed - now using Filter ADSR and Amp ADSR instead

    // Configure accent slider
    accentSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    accentSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(accentSlider);
    accentLabel.setText("Accent", juce::dontSendNotification);
    accentLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(accentLabel);
    accentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "accent", accentSlider);

    // Configure waveform slider
    waveformSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    waveformSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(waveformSlider);
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(waveformLabel);
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "waveform", waveformSlider);

    // Configure sub-oscillator slider
    subOscSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    subOscSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(subOscSlider);
    subOscLabel.setText("Sub Osc", juce::dontSendNotification);
    subOscLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subOscLabel);
    subOscAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "subosc", subOscSlider);

    // Configure drive slider
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(driveLabel);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drive", driveSlider);

    // Configure volume slider
    volumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(volumeSlider);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "volume", volumeSlider);

    // Configure Delay Time selector (IDs must start at 1, not 0 - JUCE requirement)
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

    // Configure Filter Feedback slider
    filterFeedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterFeedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterFeedbackSlider);
    filterFeedbackLabel.setText("Filter FB", juce::dontSendNotification);
    filterFeedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterFeedbackLabel);
    filterFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterfeedback", filterFeedbackSlider);

    // Configure Saturation Type selector (IDs must start at 1, not 0 - JUCE requirement)
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
    filterAttackLabel.setText("F Attack", juce::dontSendNotification);
    filterAttackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterAttackLabel);
    filterAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterattack", filterAttackSlider);

    filterDecaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterDecaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterDecaySlider);
    filterDecayLabel.setText("F Decay", juce::dontSendNotification);
    filterDecayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterDecayLabel);
    filterDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterdecay", filterDecaySlider);

    filterSustainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterSustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterSustainSlider);
    filterSustainLabel.setText("F Sustain", juce::dontSendNotification);
    filterSustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterSustainLabel);
    filterSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filtersustain", filterSustainSlider);

    filterReleaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    filterReleaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(filterReleaseSlider);
    filterReleaseLabel.setText("F Release", juce::dontSendNotification);
    filterReleaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(filterReleaseLabel);
    filterReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "filterrelease", filterReleaseSlider);

    // Configure Amplitude ADSR sliders
    ampAttackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampAttackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampAttackSlider);
    ampAttackLabel.setText("A Attack", juce::dontSendNotification);
    ampAttackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampAttackLabel);
    ampAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampattack", ampAttackSlider);

    ampDecaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampDecaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampDecaySlider);
    ampDecayLabel.setText("A Decay", juce::dontSendNotification);
    ampDecayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampDecayLabel);
    ampDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampdecay", ampDecaySlider);

    ampSustainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampSustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampSustainSlider);
    ampSustainLabel.setText("A Sustain", juce::dontSendNotification);
    ampSustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampSustainLabel);
    ampSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampsustain", ampSustainSlider);

    ampReleaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampReleaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampReleaseSlider);
    ampReleaseLabel.setText("A Release", juce::dontSendNotification);
    ampReleaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampReleaseLabel);
    ampReleaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "amprelease", ampReleaseSlider);
}

SynthTab::~SynthTab()
{
}

//==============================================================================
void SynthTab::paint(juce::Graphics& g)
{
    // Draw panel background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw section backgrounds
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(20, 60, getWidth() - 40, 180, 10); // Oscillator & Amp ADSR section
    g.fillRoundedRectangle(20, 255, getWidth() - 40, 280, 10); // Filter & Filter ADSR section
    g.fillRoundedRectangle(20, 550, getWidth() - 40, 100, 10); // Delay section

    // Draw section labels
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);
    g.drawText("OSCILLATOR & AMP ENVELOPE", 30, 65, 250, 20, juce::Justification::left);
    g.drawText("FILTER & FILTER ENVELOPE", 30, 260, 250, 20, juce::Justification::left);
    g.drawText("DELAY", 30, 555, 200, 20, juce::Justification::left);
}

void SynthTab::resized()
{
    const int knobSize = 60;
    const int labelHeight = 20;
    const int spacing = 100;

    // Preset selector at the top
    presetLabel.setBounds(getWidth() - 220, 15, 60, 20);
    presetSelector.setBounds(getWidth() - 150, 15, 130, 25);

    // Oscillator section (now first)
    int oscY = 90;
    waveformLabel.setBounds(50, oscY + knobSize, knobSize, labelHeight);
    waveformSlider.setBounds(50, oscY, knobSize, knobSize);

    subOscLabel.setBounds(50 + spacing, oscY + knobSize, knobSize, labelHeight);
    subOscSlider.setBounds(50 + spacing, oscY, knobSize, knobSize);

    driveLabel.setBounds(50 + spacing * 2, oscY + knobSize, knobSize, labelHeight);
    driveSlider.setBounds(50 + spacing * 2, oscY, knobSize, knobSize);

    volumeLabel.setBounds(50 + spacing * 3, oscY + knobSize, knobSize, labelHeight);
    volumeSlider.setBounds(50 + spacing * 3, oscY, knobSize, knobSize);

    // Second row of Oscillator section - Amplitude ADSR
    int oscY2 = oscY + 90;
    ampAttackLabel.setBounds(50, oscY2 + knobSize, knobSize, labelHeight);
    ampAttackSlider.setBounds(50, oscY2, knobSize, knobSize);

    ampDecayLabel.setBounds(50 + spacing, oscY2 + knobSize, knobSize, labelHeight);
    ampDecaySlider.setBounds(50 + spacing, oscY2, knobSize, knobSize);

    ampSustainLabel.setBounds(50 + spacing * 2, oscY2 + knobSize, knobSize, labelHeight);
    ampSustainSlider.setBounds(50 + spacing * 2, oscY2, knobSize, knobSize);

    ampReleaseLabel.setBounds(50 + spacing * 3, oscY2 + knobSize, knobSize, labelHeight);
    ampReleaseSlider.setBounds(50 + spacing * 3, oscY2, knobSize, knobSize);

    // Filter & Filter Envelope section (now second)
    int filterY = 285;
    cutoffLabel.setBounds(50, filterY + knobSize, knobSize, labelHeight);
    cutoffSlider.setBounds(50, filterY, knobSize, knobSize);

    resonanceLabel.setBounds(50 + spacing, filterY + knobSize, knobSize, labelHeight);
    resonanceSlider.setBounds(50 + spacing, filterY, knobSize, knobSize);

    envModLabel.setBounds(50 + spacing * 2, filterY + knobSize, knobSize, labelHeight);
    envModSlider.setBounds(50 + spacing * 2, filterY, knobSize, knobSize);

    accentLabel.setBounds(50 + spacing * 3, filterY + knobSize, knobSize, labelHeight);
    accentSlider.setBounds(50 + spacing * 3, filterY, knobSize, knobSize);

    // Second row of Filter section - Feedback & Saturation
    int filterY2 = filterY + 90;
    filterFeedbackLabel.setBounds(50, filterY2 + knobSize, knobSize, labelHeight);
    filterFeedbackSlider.setBounds(50, filterY2, knobSize, knobSize);

    saturationTypeLabel.setBounds(50 + spacing, filterY2 + knobSize, knobSize, labelHeight);
    saturationTypeSelector.setBounds(50 + spacing + 10, filterY2 + 25, 60, 25);

    // Third row of Filter section - Filter ADSR
    int filterY3 = filterY2 + 90;
    filterAttackLabel.setBounds(50, filterY3 + knobSize, knobSize, labelHeight);
    filterAttackSlider.setBounds(50, filterY3, knobSize, knobSize);

    filterDecayLabel.setBounds(50 + spacing, filterY3 + knobSize, knobSize, labelHeight);
    filterDecaySlider.setBounds(50 + spacing, filterY3, knobSize, knobSize);

    filterSustainLabel.setBounds(50 + spacing * 2, filterY3 + knobSize, knobSize, labelHeight);
    filterSustainSlider.setBounds(50 + spacing * 2, filterY3, knobSize, knobSize);

    filterReleaseLabel.setBounds(50 + spacing * 3, filterY3 + knobSize, knobSize, labelHeight);
    filterReleaseSlider.setBounds(50 + spacing * 3, filterY3, knobSize, knobSize);

    // Delay section
    int delayY = 580;
    delayTimeLabel.setBounds(50, delayY + knobSize, knobSize, labelHeight);
    delayTimeSelector.setBounds(50 + 10, delayY + 25, 60, 25);

    delayFeedbackLabel.setBounds(50 + spacing, delayY + knobSize, knobSize, labelHeight);
    delayFeedbackSlider.setBounds(50 + spacing, delayY, knobSize, knobSize);

    delayMixLabel.setBounds(50 + spacing * 2, delayY + knobSize, knobSize, labelHeight);
    delayMixSlider.setBounds(50 + spacing * 2, delayY, knobSize, knobSize);
}
