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

    // Configure Amplitude ADSR sliders
    ampAttackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampAttackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampAttackSlider);
    ampAttackLabel.setText("Attack", juce::dontSendNotification);
    ampAttackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampAttackLabel);
    ampAttackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampattack", ampAttackSlider);

    ampDecaySlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampDecaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampDecaySlider);
    ampDecayLabel.setText("Decay", juce::dontSendNotification);
    ampDecayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampDecayLabel);
    ampDecayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampdecay", ampDecaySlider);

    ampSustainSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampSustainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampSustainSlider);
    ampSustainLabel.setText("Sustain", juce::dontSendNotification);
    ampSustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(ampSustainLabel);
    ampSustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "ampsustain", ampSustainSlider);

    ampReleaseSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    ampReleaseSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(ampReleaseSlider);
    ampReleaseLabel.setText("Release", juce::dontSendNotification);
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

    const int boxGap = 10; // Gap between boxes
    const int box1Y = 60;
    const int box1Height = 200;
    const int box2Y = box1Y + box1Height + boxGap;
    const int box2Height = 200;
    const int box3Y = box2Y + box2Height + boxGap;
    const int box3Height = 110;

    // Draw section backgrounds
    g.setColour(juce::Colour(0xff3a3a3a));
    g.fillRoundedRectangle(20, box1Y, getWidth() - 40, box1Height, 10); // Oscillator & Amp ADSR section
    g.fillRoundedRectangle(20, box2Y, getWidth() - 40, box2Height, 10); // Filter & Filter ADSR section
    g.fillRoundedRectangle(20, box3Y, getWidth() - 40, box3Height, 10); // Delay section

    // Draw section labels
    g.setColour(juce::Colours::lightgrey);
    g.setFont(14.0f);
    g.drawText("OSCILLATOR & AMP ENVELOPE", 30, box1Y + 5, 250, 20, juce::Justification::left);
    g.drawText("FILTER & FILTER ENVELOPE", 30, box2Y + 5, 250, 20, juce::Justification::left);
    g.drawText("DELAY", 30, box3Y + 5, 200, 20, juce::Justification::left);
}

void SynthTab::resized()
{
    const int knobSize = 60;
    const int labelHeight = 20;
    const int columnSpacing = 130;  // Uniform column spacing across all boxes
    const int startX = 50;

    // Column position calculator
    auto getColumnX = [&](int col) { return startX + col * columnSpacing; };

    // Preset selector at the top
    presetLabel.setBounds(getWidth() - 220, 15, 60, 20);
    presetSelector.setBounds(getWidth() - 150, 15, 130, 25);

    // BOX 1: OSCILLATOR & AMP ENVELOPE
    // Row 1: Waveform, SubOsc, Drive, Volume (columns 0-3)
    int box1Row1Y = 90;
    waveformLabel.setBounds(getColumnX(0), box1Row1Y + knobSize, knobSize, labelHeight);
    waveformSlider.setBounds(getColumnX(0), box1Row1Y, knobSize, knobSize);

    subOscLabel.setBounds(getColumnX(1), box1Row1Y + knobSize, knobSize, labelHeight);
    subOscSlider.setBounds(getColumnX(1), box1Row1Y, knobSize, knobSize);

    driveLabel.setBounds(getColumnX(2), box1Row1Y + knobSize, knobSize, labelHeight);
    driveSlider.setBounds(getColumnX(2), box1Row1Y, knobSize, knobSize);

    volumeLabel.setBounds(getColumnX(3), box1Row1Y + knobSize, knobSize, labelHeight);
    volumeSlider.setBounds(getColumnX(3), box1Row1Y, knobSize, knobSize);

    // Row 2: Attack, Decay, Sustain, Release (Amp ADSR - columns 0-3)
    int box1Row2Y = box1Row1Y + 90;
    ampAttackLabel.setBounds(getColumnX(0), box1Row2Y + knobSize, knobSize, labelHeight);
    ampAttackSlider.setBounds(getColumnX(0), box1Row2Y, knobSize, knobSize);

    ampDecayLabel.setBounds(getColumnX(1), box1Row2Y + knobSize, knobSize, labelHeight);
    ampDecaySlider.setBounds(getColumnX(1), box1Row2Y, knobSize, knobSize);

    ampSustainLabel.setBounds(getColumnX(2), box1Row2Y + knobSize, knobSize, labelHeight);
    ampSustainSlider.setBounds(getColumnX(2), box1Row2Y, knobSize, knobSize);

    ampReleaseLabel.setBounds(getColumnX(3), box1Row2Y + knobSize, knobSize, labelHeight);
    ampReleaseSlider.setBounds(getColumnX(3), box1Row2Y, knobSize, knobSize);

    // BOX 2: FILTER & FILTER ENVELOPE
    // Row 1: Cutoff, Resonance, EnvMod, Accent, Filter FB, Saturation (columns 0-5)
    int box2Row1Y = 300;
    cutoffLabel.setBounds(getColumnX(0), box2Row1Y + knobSize, knobSize, labelHeight);
    cutoffSlider.setBounds(getColumnX(0), box2Row1Y, knobSize, knobSize);

    resonanceLabel.setBounds(getColumnX(1), box2Row1Y + knobSize, knobSize, labelHeight);
    resonanceSlider.setBounds(getColumnX(1), box2Row1Y, knobSize, knobSize);

    envModLabel.setBounds(getColumnX(2), box2Row1Y + knobSize, knobSize, labelHeight);
    envModSlider.setBounds(getColumnX(2), box2Row1Y, knobSize, knobSize);

    accentLabel.setBounds(getColumnX(3), box2Row1Y + knobSize, knobSize, labelHeight);
    accentSlider.setBounds(getColumnX(3), box2Row1Y, knobSize, knobSize);

    filterFeedbackLabel.setBounds(getColumnX(4), box2Row1Y + knobSize, knobSize, labelHeight);
    filterFeedbackSlider.setBounds(getColumnX(4), box2Row1Y, knobSize, knobSize);

    saturationTypeLabel.setBounds(getColumnX(5), box2Row1Y + knobSize, knobSize, labelHeight);
    saturationTypeSelector.setBounds(getColumnX(5) + 10, box2Row1Y + 25, 60, 25);

    // Row 2: Attack, Decay, Sustain, Release (Filter ADSR - columns 0-3)
    int box2Row2Y = box2Row1Y + 90;
    filterAttackLabel.setBounds(getColumnX(0), box2Row2Y + knobSize, knobSize, labelHeight);
    filterAttackSlider.setBounds(getColumnX(0), box2Row2Y, knobSize, knobSize);

    filterDecayLabel.setBounds(getColumnX(1), box2Row2Y + knobSize, knobSize, labelHeight);
    filterDecaySlider.setBounds(getColumnX(1), box2Row2Y, knobSize, knobSize);

    filterSustainLabel.setBounds(getColumnX(2), box2Row2Y + knobSize, knobSize, labelHeight);
    filterSustainSlider.setBounds(getColumnX(2), box2Row2Y, knobSize, knobSize);

    filterReleaseLabel.setBounds(getColumnX(3), box2Row2Y + knobSize, knobSize, labelHeight);
    filterReleaseSlider.setBounds(getColumnX(3), box2Row2Y, knobSize, knobSize);

    // BOX 3: DELAY
    // Row 1: Delay Time, Delay FB, Delay Mix (columns 0-2)
    int box3Row1Y = 510;
    delayTimeLabel.setBounds(getColumnX(0), box3Row1Y + knobSize, knobSize, labelHeight);
    delayTimeSelector.setBounds(getColumnX(0) + 10, box3Row1Y + 25, 60, 25);

    delayFeedbackLabel.setBounds(getColumnX(1), box3Row1Y + knobSize, knobSize, labelHeight);
    delayFeedbackSlider.setBounds(getColumnX(1), box3Row1Y, knobSize, knobSize);

    delayMixLabel.setBounds(getColumnX(2), box3Row1Y + knobSize, knobSize, labelHeight);
    delayMixSlider.setBounds(getColumnX(2), box3Row1Y, knobSize, knobSize);
}
