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

    // Add tabs to tabbed component
    tabbedComponent.addTab("Synth", juce::Colour(0xff2a2a2a), synthTab.get(), false);
    tabbedComponent.addTab("Modulation", juce::Colour(0xff2a2a2a), modulationTab.get(), false);
    tabbedComponent.addTab("Sequencer", juce::Colour(0xff2a2a2a), sequencerTab.get(), false);

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
            audioProcessor.startInternalPlayback();
        }
        else
        {
            playStopButton.setButtonText("PLAY");
            audioProcessor.stopInternalPlayback();
        }
    };
    addAndMakeVisible(playStopButton);
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
}

void AcidSynthAudioProcessorEditor::resized()
{
    // Position the Play/Stop button in the title area
    playStopButton.setBounds(20, 15, 80, 30);

    // Position the tabbed component below the title
    tabbedComponent.setBounds(0, 50, getWidth(), getHeight() - 50);
}
