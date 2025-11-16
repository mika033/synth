#include "PluginProcessor.h"
#include "SynthTab.h"

//==============================================================================
SynthTab::SynthTab(AcidSynthAudioProcessor& p)
    : audioProcessor(p)
{
    // Configure preset selector
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
    cutoffLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(cutoffLabel);
    cutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "cutoff", cutoffSlider);

    // Configure resonance slider
    resonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(resonanceSlider);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(resonanceLabel);
    resonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "resonance", resonanceSlider);

    // Configure envelope mod slider
    envModSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    envModSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(envModSlider);
    envModLabel.setText("Env Mod", juce::dontSendNotification);
    envModLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(envModLabel);
    envModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "envmod", envModSlider);

    // Configure decay slider
    decaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    decaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(decaySlider);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(decayLabel);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "decay", decaySlider);

    // Configure accent slider
    accentSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    accentSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(accentSlider);
    accentLabel.setText("Accent", juce::dontSendNotification);
    accentLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(accentLabel);
    accentAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "accent", accentSlider);

    // Configure waveform slider
    waveformSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    waveformSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(waveformSlider);
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(waveformLabel);
    waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "waveform", waveformSlider);

    // Configure sub-oscillator slider
    subOscSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    subOscSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(subOscSlider);
    subOscLabel.setText("Sub Osc", juce::dontSendNotification);
    subOscLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subOscLabel);
    subOscAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "subosc", subOscSlider);

    // Configure drive slider
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(driveSlider);
    driveLabel.setText("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(driveLabel);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "drive", driveSlider);

    // Configure volume slider
    volumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(volumeSlider);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "volume", volumeSlider);

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
    delayTimeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(delayTimeLabel);
    delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), "delaytime", delayTimeSelector);

    // Configure Delay Feedback slider
    delayFeedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayFeedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayFeedbackSlider);
    delayFeedbackLabel.setText("Delay FB", juce::dontSendNotification);
    delayFeedbackLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(delayFeedbackLabel);
    delayFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "delayfeedback", delayFeedbackSlider);

    // Configure Delay Mix slider
    delayMixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    delayMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(delayMixSlider);
    delayMixLabel.setText("Delay Mix", juce::dontSendNotification);
    delayMixLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(delayMixLabel);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "delaymix", delayMixSlider);

    // Setup all 10 dedicated LFO control sets
    setupLFOControls(cutoffLFO, "cutofflfor", "cutofflfow", "cutofflfod");
    setupLFOControls(resonanceLFO, "resonancelfor", "resonancelfow", "resonancelfod");
    setupLFOControls(envModLFO, "envmodlfor", "envmodlfow", "envmodlfod");
    setupLFOControls(decayLFO, "decaylfor", "decaylfow", "decaylfod");
    setupLFOControls(accentLFO, "accentlfor", "accentlfow", "accentlfod");
    setupLFOControls(waveformLFO, "waveformlfor", "waveformlfow", "waveformlfod");
    setupLFOControls(subOscLFO, "subosclfor", "subosclfow", "subosclfod");
    setupLFOControls(driveLFO, "drivelfor", "drivelfow", "drivelfod");
    setupLFOControls(volumeLFO, "volumelfor", "volumelfow", "volumelfod");
    setupLFOControls(delayMixLFO, "delaymixlfor", "delaymixlfow", "delaymixlfod");
}

SynthTab::~SynthTab()
{
}

void SynthTab::setupLFOControls(LFOControls& lfo,
                                 const juce::String& rateParamID,
                                 const juce::String& waveParamID,
                                 const juce::String& depthParamID)
{
    // Setup LFO Rate selector
    lfo.rateSelector.addItem("1/16", 1);
    lfo.rateSelector.addItem("1/8", 2);
    lfo.rateSelector.addItem("1/4", 3);
    lfo.rateSelector.addItem("1/3", 4);
    lfo.rateSelector.addItem("1/2", 5);
    lfo.rateSelector.addItem("3/4", 6);
    lfo.rateSelector.addItem("1/1", 7);
    lfo.rateSelector.addItem("2/1", 8);
    lfo.rateSelector.addItem("3/1", 9);
    lfo.rateSelector.addItem("4/1", 10);
    lfo.rateSelector.addItem("6/1", 11);
    lfo.rateSelector.addItem("8/1", 12);
    lfo.rateSelector.addItem("12/1", 13);
    lfo.rateSelector.addItem("16/1", 14);
    addAndMakeVisible(lfo.rateSelector);
    lfo.rateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), rateParamID, lfo.rateSelector);

    // Setup LFO Waveform selector
    lfo.waveformSelector.addItem("Sine", 1);
    lfo.waveformSelector.addItem("Triangle", 2);
    lfo.waveformSelector.addItem("Saw Up", 3);
    lfo.waveformSelector.addItem("Saw Dn", 4);
    lfo.waveformSelector.addItem("Square", 5);
    lfo.waveformSelector.addItem("Random", 6);
    addAndMakeVisible(lfo.waveformSelector);
    lfo.waveformAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.getValueTreeState(), waveParamID, lfo.waveformSelector);

    // Setup LFO Depth slider
    lfo.depthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    lfo.depthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(lfo.depthSlider);
    lfo.depthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), depthParamID, lfo.depthSlider);
}

//==============================================================================
void SynthTab::paint(juce::Graphics& g)
{
    // Draw panel background
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Draw row separators
    g.setColour(juce::Colour(0xff3a3a3a));
    for (int i = 1; i <= 10; ++i)
    {
        int y = 5 + i * 60;
        g.drawLine(10.0f, static_cast<float>(y), static_cast<float>(getWidth() - 10), static_cast<float>(y), 1.0f);
    }

    // Draw LFO section labels
    g.setColour(juce::Colours::lightgrey);
    g.setFont(juce::Font(9.0f));
    int lfoHeaderX = 200;
    g.drawText("--- LFO Controls ---", lfoHeaderX, 13, 600, 15, juce::Justification::centredLeft);
}

void SynthTab::resized()
{
    const int knobSize = 50;  // All knobs same size
    const int rowHeight = 60;
    const int startY = 20;
    const int paramLabelX = 20;
    const int paramKnobX = 120;
    const int lfoStartX = 200;
    const int lfoSpacing = 120;

    // Preset selector at the top
    presetLabel.setBounds(getWidth() - 220, 5, 60, 20);
    presetSelector.setBounds(getWidth() - 150, 5, 130, 25);

    // Row-based layout helper lambda
    auto layoutRow = [&](int rowIndex, juce::Slider& paramSlider, juce::Label& paramLabel, LFOControls& lfo)
    {
        int y = startY + rowIndex * rowHeight;

        // Main parameter
        paramLabel.setBounds(paramLabelX, y + 10, 90, 20);
        paramSlider.setBounds(paramKnobX, y, knobSize, knobSize);

        // LFO Rate selector
        lfo.rateSelector.setBounds(lfoStartX, y + 12, 80, 26);

        // LFO Waveform selector
        lfo.waveformSelector.setBounds(lfoStartX + lfoSpacing, y + 12, 80, 26);

        // LFO Depth knob
        lfo.depthSlider.setBounds(lfoStartX + lfoSpacing * 2, y, knobSize, knobSize);
    };

    // Layout all 10 rows with dedicated LFO controls
    layoutRow(0, cutoffSlider, cutoffLabel, cutoffLFO);
    layoutRow(1, resonanceSlider, resonanceLabel, resonanceLFO);
    layoutRow(2, envModSlider, envModLabel, envModLFO);
    layoutRow(3, decaySlider, decayLabel, decayLFO);
    layoutRow(4, accentSlider, accentLabel, accentLFO);
    layoutRow(5, waveformSlider, waveformLabel, waveformLFO);
    layoutRow(6, subOscSlider, subOscLabel, subOscLFO);
    layoutRow(7, driveSlider, driveLabel, driveLFO);
    layoutRow(8, volumeSlider, volumeLabel, volumeLFO);

    // Row 9: Delay Time and Delay Feedback (no LFOs for these)
    int delayRow1Y = startY + 9 * rowHeight;
    delayTimeLabel.setBounds(paramLabelX, delayRow1Y + 10, 90, 20);
    delayTimeSelector.setBounds(paramKnobX, delayRow1Y + 12, 80, 26);

    delayFeedbackLabel.setBounds(lfoStartX, delayRow1Y + 10, 90, 20);
    delayFeedbackSlider.setBounds(lfoStartX + 90, delayRow1Y, knobSize, knobSize);

    // Row 10: Delay Mix with LFO controls
    layoutRow(10, delayMixSlider, delayMixLabel, delayMixLFO);
}
