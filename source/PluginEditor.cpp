#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SnorkelSynthAudioProcessorEditor::SnorkelSynthAudioProcessorEditor(SnorkelSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    // Register this editor with the processor
    audioProcessor.setEditor(this);

    // Set editor size
    setSize(900, 700);

    // Create tab components
    oscTab = std::make_unique<OscTab>(audioProcessor, *this);
    filterTab = std::make_unique<FilterTab>(audioProcessor);
    sequencerTab = std::make_unique<SequencerTab>(audioProcessor);
    modulationTab = std::make_unique<ModulationTab>(audioProcessor);
    melodySequencerTab = std::make_unique<MelodySequencerTab>(audioProcessor);
    progressionTab = std::make_unique<ProgressionTab>(audioProcessor);

    // Add tabs to tabbed component
    tabbedComponent.addTab("Osc", juce::Colour(0xff2a2a2a), oscTab.get(), false);
    tabbedComponent.addTab("Filter", juce::Colour(0xff2a2a2a), filterTab.get(), false);
    tabbedComponent.addTab("Modulation", juce::Colour(0xff2a2a2a), modulationTab.get(), false);
    tabbedComponent.addTab("Sequencer", juce::Colour(0xff2a2a2a), melodySequencerTab.get(), false);
    tabbedComponent.addTab("Arpeggiator", juce::Colour(0xff2a2a2a), sequencerTab.get(), false);
    tabbedComponent.addTab("Progression", juce::Colour(0xff2a2a2a), progressionTab.get(), false);

    // Configure tabbed component appearance
    tabbedComponent.setTabBarDepth(35);
    tabbedComponent.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff1a1a1a));
    tabbedComponent.setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff4a4a4a));

    // Show Sequencer tab at startup (index 3, was index 2)
    tabbedComponent.setCurrentTabIndex(3);

    addAndMakeVisible(tabbedComponent);

    // Configure Play/Stop button
    playStopButton.setButtonText("PLAY");
    playStopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff00aa00));
    playStopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xffaa0000));
    playStopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    playStopButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    playStopButton.setClickingTogglesState(true);
    playStopButton.onClick = [this]
    {
        if (playStopButton.getToggleState())
        {
            playStopButton.setButtonText("STOP");
            audioProcessor.startPlayback();
        }
        else
        {
            playStopButton.setButtonText("PLAY");
            audioProcessor.stopPlayback();
        }
    };
    addAndMakeVisible(playStopButton);

    // Configure BPM slider (text box only, drag to change)
    bpmSlider.setSliderStyle(juce::Slider::IncDecButtons);
    bpmSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 50, 20);
    bpmSlider.setRange(60.0, 200.0, 1.0);
    bpmSlider.setValue(120.0);
    bpmSlider.setIncDecButtonsMode(juce::Slider::incDecButtonsDraggable_Vertical);
    addAndMakeVisible(bpmSlider);

    bpmLabel.setText("BPM", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centredRight);
    bpmLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(bpmLabel);

    // Configure Volume slider (no text box)
    masterVolumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    masterVolumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterVolumeSlider.setRange(0.0, 1.0, 0.01);
    masterVolumeSlider.setValue(0.7);
    addAndMakeVisible(masterVolumeSlider);

    masterVolumeLabel.setText("Volume", juce::dontSendNotification);
    masterVolumeLabel.setJustificationType(juce::Justification::centredRight);
    masterVolumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(masterVolumeLabel);

    // Configure message display label
    messageLabel.setText("", juce::dontSendNotification);
    messageLabel.setJustificationType(juce::Justification::centredLeft);
    messageLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    messageLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(messageLabel);

    // Attach to parameters
    bpmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "globalbpm", bpmSlider);
    masterVolumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "mastervolume", masterVolumeSlider);
}

SnorkelSynthAudioProcessorEditor::~SnorkelSynthAudioProcessorEditor()
{
    stopTimer();
    // Unregister this editor from the processor
    audioProcessor.setEditor(nullptr);
}

void SnorkelSynthAudioProcessorEditor::showMessage(const juce::String& message)
{
    messageLabel.setText(message, juce::dontSendNotification);
    startTimer(2000); // Clear message after 2 seconds
}

void SnorkelSynthAudioProcessorEditor::timerCallback()
{
    messageLabel.setText("", juce::dontSendNotification);
    stopTimer();
}

//==============================================================================
void SnorkelSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void SnorkelSynthAudioProcessorEditor::resized()
{
    // Top bar layout from left to right: Play/Stop | Volume | Message | BPM
    const int topY = 15;
    const int topHeight = 30;
    int x = 20;

    // Play/Stop button at the left
    playStopButton.setBounds(x, topY, 80, topHeight);
    x += 80 + 20;

    // Volume control
    masterVolumeLabel.setBounds(x, topY, 60, topHeight);
    x += 65;
    masterVolumeSlider.setBounds(x, topY, 120, topHeight);
    x += 120 + 20;

    // Message label (takes remaining space before BPM)
    int bpmWidth = 105; // BPM label (40) + gap + BPM slider (60)
    int bpmStartX = getWidth() - bpmWidth - 20;
    messageLabel.setBounds(x, topY, bpmStartX - x - 20, topHeight);

    // BPM control at the right
    bpmLabel.setBounds(bpmStartX, topY, 40, topHeight);
    bpmSlider.setBounds(bpmStartX + 45, topY, 60, topHeight);

    // Position the tabbed component below the top bar
    tabbedComponent.setBounds(0, 50, getWidth(), getHeight() - 50);
}
