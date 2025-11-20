cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0175 NEW)

#===============================================================================
# PLUGIN CONFIGURATION - Edit these to customize your plugin
#===============================================================================

# Plugin Identity
set(PLUGIN_NAME "SnorkelSynth")            # Internal project name (no spaces)
set(PLUGIN_VERSION "1.0.0")                # Plugin version
set(PLUGIN_DISPLAY_NAME "Snorkel Synth")   # Name shown in DAW
set(COMPANY_NAME "SnorkelLab")             # Your company/developer name
set(PLUGIN_MANUFACTURER_CODE "Snkl")       # 4-char manufacturer ID (must be unique!)
set(PLUGIN_CODE "Snk1")                    # 4-char plugin ID (must be unique!)

# Build Configuration
set(PLUGIN_FORMATS "VST3" "Standalone")    # Available: VST3, AU, AAX, Standalone
set(INSTALL_TO_SYSTEM TRUE)                # TRUE = auto-install, FALSE = manual

# Output File Names (automatically generated)
# VST3: ${PLUGIN_DISPLAY_NAME}.vst3
# Standalone: ${PLUGIN_DISPLAY_NAME}.exe (Windows) or ${PLUGIN_DISPLAY_NAME}.app (Mac)

# Default VST Installation Directories:
# Windows: C:\Program Files\Common Files\VST3\
# macOS:   ~/Library/Audio/Plug-Ins/VST3/
# Linux:   ~/.vst3/
# (JUCE handles installation automatically when INSTALL_TO_SYSTEM is TRUE)

#===============================================================================
project(${PLUGIN_NAME} VERSION ${PLUGIN_VERSION})

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
juce_add_plugin(${PLUGIN_NAME}
    COMPANY_NAME ${COMPANY_NAME}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${INSTALL_TO_SYSTEM}
    PLUGIN_MANUFACTURER_CODE ${PLUGIN_MANUFACTURER_CODE}
    PLUGIN_CODE ${PLUGIN_CODE}
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME ${PLUGIN_DISPLAY_NAME}
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/icon.png"
)

# Source files
target_sources(${PLUGIN_NAME} PRIVATE
    source/PluginProcessor.cpp
    source/PluginEditor.cpp
    source/AcidVoice.cpp
    source/OscTab.cpp
    source/FilterTab.cpp
    source/SequencerTab.cpp
    source/ModulationTab.cpp
    source/MelodySequencerTab.cpp
    source/ProgressionTab.cpp
)

# Header files
target_include_directories(${PLUGIN_NAME} PRIVATE
    include
)

# JUCE modules
target_compile_definitions(${PLUGIN_NAME} PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(${PLUGIN_NAME} PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
)
