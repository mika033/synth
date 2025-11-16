#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
/**
 * Modulation Tab - Contains all LFO modulation controls
 */
class ModulationTab : public juce::Component
{
public:
    ModulationTab(AcidSynthAudioProcessor& p);
    ~ModulationTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    AcidSynthAudioProcessor& audioProcessor;

    // Helper structure for LFO controls (each parameter gets one)
    struct LFOControls
    {
        juce::ComboBox rateSelector;
        juce::ComboBox waveformSelector;
        juce::Slider depthSlider;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> rateAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveformAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    };

    // Parameter labels for each row
    juce::Label cutoffLabel;
    juce::Label resonanceLabel;
    juce::Label envModLabel;
    juce::Label decayLabel;
    juce::Label accentLabel;
    juce::Label waveformLabel;
    juce::Label subOscLabel;
    juce::Label driveLabel;
    juce::Label volumeLabel;
    juce::Label delayMixLabel;

    // 10 Dedicated LFO control sets
    LFOControls cutoffLFO;
    LFOControls resonanceLFO;
    LFOControls envModLFO;
    LFOControls decayLFO;
    LFOControls accentLFO;
    LFOControls waveformLFO;
    LFOControls subOscLFO;
    LFOControls driveLFO;
    LFOControls volumeLFO;
    LFOControls delayMixLFO;

    // Helper function to setup LFO controls
    void setupLFOControls(LFOControls& lfo, const juce::String& rateParamID,
                          const juce::String& waveParamID, const juce::String& depthParamID);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationTab)
};
