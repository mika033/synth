#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AcidSynthAudioProcessorEditor::AcidSynthAudioProcessorEditor(AcidSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    // Set editor size
    setSize(900, 700);

    // Create tab components
    synthTab = std::make_unique<SynthTab>(audioProcessor);
    sequencerTab = std::make_unique<SequencerTab>(audioProcessor);
    modulationTab = std::make_unique<ModulationTab>(audioProcessor);
    melodySequencerTab = std::make_unique<MelodySequencerTab>(audioProcessor);
    progressionTab = std::make_unique<ProgressionTab>(audioProcessor);

    // Add tabs to tabbed component
    tabbedComponent.addTab("Synth", juce::Colour(0xff2a2a2a), synthTab.get(), false);
    tabbedComponent.addTab("Modulation", juce::Colour(0xff2a2a2a), modulationTab.get(), false);
    tabbedComponent.addTab("Sequencer", juce::Colour(0xff2a2a2a), melodySequencerTab.get(), false);
    tabbedComponent.addTab("Arpeggiator", juce::Colour(0xff2a2a2a), sequencerTab.get(), false);
    tabbedComponent.addTab("Progression", juce::Colour(0xff2a2a2a), progressionTab.get(), false);

    // Configure tabbed component appearance
    tabbedComponent.setTabBarDepth(35);
    tabbedComponent.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff1a1a1a));
    tabbedComponent.setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff4a4a4a));

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

    // Attach to parameters
    bpmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "globalbpm", bpmSlider);
    masterVolumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.getValueTreeState(), "mastervolume", masterVolumeSlider);
}

AcidSynthAudioProcessorEditor::~AcidSynthAudioProcessorEditor()
{
}

//==============================================================================
void AcidSynthAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void AcidSynthAudioProcessorEditor::resized()
{
    // Position the Play/Stop button in the top left
    playStopButton.setBounds(20, 15, 80, 30);

    // Position BPM control to the right of the screen (just text box)
    int rightSideX = getWidth() - 120;
    bpmLabel.setBounds(rightSideX - 45, 15, 40, 30);
    bpmSlider.setBounds(rightSideX, 15, 60, 30);

    // Position Volume control next to BPM
    int volumeX = rightSideX - 220;
    masterVolumeLabel.setBounds(volumeX, 15, 60, 30);
    masterVolumeSlider.setBounds(volumeX + 65, 15, 120, 30);

    // Position the tabbed component below the top bar
    tabbedComponent.setBounds(0, 50, getWidth(), getHeight() - 50);
}
