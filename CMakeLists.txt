cmake_minimum_required(VERSION 3.15)

project(AcidSynth VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch JUCE framework
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.9
)
FetchContent_MakeAvailable(JUCE)

# Plugin configuration
juce_add_plugin(AcidSynth
    COMPANY_NAME "AcidLab"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Acdl
    PLUGIN_CODE Acd1
    FORMATS VST3 Standalone
    PRODUCT_NAME "Acid Synth"
)

# Source files
target_sources(AcidSynth PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/AcidVoice.cpp
)

# Header files
target_include_directories(AcidSynth PRIVATE
    include
)

# JUCE modules
target_compile_definitions(AcidSynth PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(AcidSynth PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
)
