#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "SynthTab.h"
#include "SequencerTab.h"
#include "ModulationTab.h"

//==============================================================================
/**
 * UI Editor for Acid Synth plugin with tabbed interface
 */
class AcidSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    AcidSynthAudioProcessorEditor(AcidSynthAudioProcessor&);
    ~AcidSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& audioProcessor;

    // Tabbed component
    juce::TabbedComponent tabbedComponent;

    // Tab components
    std::unique_ptr<SynthTab> synthTab;
    std::unique_ptr<SequencerTab> sequencerTab;
    std::unique_ptr<ModulationTab> modulationTab;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AcidSynthAudioProcessorEditor)
};
