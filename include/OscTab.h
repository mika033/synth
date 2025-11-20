#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

// Forward declaration
class SnorkelSynthAudioProcessorEditor;

//==============================================================================
/**
 * Oscillator Tab - Contains oscillator and amplitude controls
 */
class OscTab : public juce::Component
{
public:
    OscTab(SnorkelSynthAudioProcessor& p, SnorkelSynthAudioProcessorEditor& e);
    ~OscTab() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SnorkelSynthAudioProcessor& audioProcessor;
    SnorkelSynthAudioProcessorEditor& editor;

    // Sub-tabs for oscillators
    juce::TabbedComponent oscTabs;

    // Three oscillators - each has Wave, Coarse, Fine, Mix
    juce::Slider osc1WaveSlider, osc1CoarseSlider, osc1FineSlider, osc1MixSlider;
    juce::Slider osc2WaveSlider, osc2CoarseSlider, osc2FineSlider, osc2MixSlider;
    juce::Slider osc3WaveSlider, osc3CoarseSlider, osc3FineSlider, osc3MixSlider;

    // Noise oscillator (order: Type, Decay, Volume)
    juce::Slider noiseTypeSlider;
    juce::Slider noiseDecaySlider;
    juce::Slider noiseMixSlider;

    // Global controls
    juce::Slider driveSlider;
    juce::Slider volumeSlider;
    juce::Slider globalOctaveSlider;

    // Analog character controls
    juce::Slider driftSlider;
    juce::Slider phaseRandomSlider;
    juce::Slider unisonSlider;

    // Amplitude ADSR sliders
    juce::Slider ampAttackSlider;
    juce::Slider ampDecaySlider;
    juce::Slider ampSustainSlider;
    juce::Slider ampReleaseSlider;

    // Preset controls
    juce::ComboBox presetSelector;
    juce::TextButton savePresetButton;

    // Oscillator labels
    juce::Label osc1WaveLabel, osc1CoarseLabel, osc1FineLabel, osc1MixLabel;
    juce::Label osc2WaveLabel, osc2CoarseLabel, osc2FineLabel, osc2MixLabel;
    juce::Label osc3WaveLabel, osc3CoarseLabel, osc3FineLabel, osc3MixLabel;

    // Noise labels
    juce::Label noiseTypeLabel;
    juce::Label noiseDecayLabel;
    juce::Label noiseMixLabel;

    // Global control labels
    juce::Label driveLabel;
    juce::Label volumeLabel;
    juce::Label globalOctaveLabel;

    // Analog character labels
    juce::Label driftLabel;
    juce::Label phaseRandomLabel;
    juce::Label unisonLabel;

    // Amplitude ADSR labels
    juce::Label ampAttackLabel;
    juce::Label ampDecayLabel;
    juce::Label ampSustainLabel;
    juce::Label ampReleaseLabel;
    juce::Label presetLabel;

    // Oscillator attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc1WaveAttachment, osc1CoarseAttachment, osc1FineAttachment, osc1MixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc2WaveAttachment, osc2CoarseAttachment, osc2FineAttachment, osc2MixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> osc3WaveAttachment, osc3CoarseAttachment, osc3FineAttachment, osc3MixAttachment;

    // Noise attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseTypeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseMixAttachment;

    // Global control attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> globalOctaveAttachment;

    // Analog character attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driftAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> phaseRandomAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> unisonAttachment;

    // Amplitude ADSR attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ampAttackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ampDecayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ampSustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ampReleaseAttachment;

    // Helper method to update feedback label
    void updateFeedback(const juce::String& paramName, float value, const juce::String& unit = "");

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscTab)
};
