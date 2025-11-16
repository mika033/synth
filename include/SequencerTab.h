#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Sequencer Tab - Contains arpeggiator controls
 */
class SequencerTab : public juce::Component
{
public:
    SequencerTab(AcidSynthAudioProcessor& p);
    ~SequencerTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& audioProcessor;

    // Arpeggiator controls
    juce::ToggleButton arpOnOffToggle;
    juce::ComboBox arpModeSelector;
    juce::ComboBox arpRateSelector;
    juce::Slider arpOctavesSlider;
    juce::Slider arpGateSlider;
    juce::Slider arpOctaveShiftSlider;

    // Labels
    juce::Label arpOnOffLabel;
    juce::Label arpModeLabel;
    juce::Label arpRateLabel;
    juce::Label arpOctavesLabel;
    juce::Label arpGateLabel;
    juce::Label arpOctaveShiftLabel;
    juce::Label titleLabel;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> arpOnOffAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> arpRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpOctavesAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpGateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> arpOctaveShiftAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SequencerTab)
};
