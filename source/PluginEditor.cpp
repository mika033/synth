#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AcidSynthAudioProcessorEditor::AcidSynthAudioProcessorEditor(AcidSynthAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    // Set editor size
    setSize(900, 820);

    // Create tab components
    synthTab = std::make_unique<SynthTab>(audioProcessor);
    sequencerTab = std::make_unique<SequencerTab>(audioProcessor);

    // Add tabs to tabbed component
    tabbedComponent.addTab("Synth", juce::Colour(0xff2a2a2a), synthTab.get(), false);
    tabbedComponent.addTab("Sequencer", juce::Colour(0xff2a2a2a), sequencerTab.get(), false);

    // Configure tabbed component appearance
    tabbedComponent.setTabBarDepth(35);
    tabbedComponent.setColour(juce::TabbedComponent::backgroundColourId, juce::Colour(0xff1a1a1a));
    tabbedComponent.setColour(juce::TabbedComponent::outlineColourId, juce::Colour(0xff4a4a4a));

    addAndMakeVisible(tabbedComponent);
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
    // Position the tabbed component below the title
    tabbedComponent.setBounds(0, 50, getWidth(), getHeight() - 50);
}
