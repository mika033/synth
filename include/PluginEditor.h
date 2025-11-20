#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "OscTab.h"
#include "FilterTab.h"
#include "SequencerTab.h"
#include "ModulationTab.h"
#include "MelodySequencerTab.h"
#include "ProgressionTab.h"

//==============================================================================
/**
 * UI Editor for Snorkel Synth plugin with tabbed interface
 */
class SnorkelSynthAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    SnorkelSynthAudioProcessorEditor(SnorkelSynthAudioProcessor&);
    ~SnorkelSynthAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    // Public method to display temporary messages
    void showMessage(const juce::String& message);

private:
    SnorkelSynthAudioProcessor& audioProcessor;

    // Tabbed component
    juce::TabbedComponent tabbedComponent;

    // Tab components
    std::unique_ptr<OscTab> oscTab;
    std::unique_ptr<FilterTab> filterTab;
    std::unique_ptr<SequencerTab> sequencerTab;
    std::unique_ptr<ModulationTab> modulationTab;
    std::unique_ptr<MelodySequencerTab> melodySequencerTab;
    std::unique_ptr<ProgressionTab> progressionTab;

    // Play/Stop button for standalone mode
    juce::TextButton playStopButton;

    // BPM and Master Volume controls
    juce::Slider bpmSlider;
    juce::Label bpmLabel;
    juce::Slider masterVolumeSlider;
    juce::Label masterVolumeLabel;

    // Message display label
    juce::Label messageLabel;

    // Parameter attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bpmAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> masterVolumeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SnorkelSynthAudioProcessorEditor)
};
