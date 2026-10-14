cmake_minimum_required(VERSION 3.22)

project(VoicePad VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.9
    GIT_SHALLOW    TRUE)

FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(VoicePad
    PRODUCT_NAME  "VoicePad"
    COMPANY_NAME  "GarciSoft"
    VERSION       ${PROJECT_VERSION}
    BUNDLE_ID     "com.garci.voicepad"
    ICON_BIG      "${CMAKE_CURRENT_SOURCE_DIR}/Assets/icon.png")

target_sources(VoicePad PRIVATE
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/Theme.cpp
    Source/UI/ConsoleLookAndFeel.cpp
    Source/UI/Panel.cpp
    Source/UI/Toolbar.cpp
    Source/UI/DevicePanel.cpp
    Source/UI/PresetPanel.cpp
    Source/UI/EffectsPanel.cpp
    Source/UI/ScopePanel.cpp
    Source/UI/SoundboardPanel.cpp
    Source/UI/ProfilesPanel.cpp
    Source/UI/TablePanel.cpp
    Source/UI/ManualPanel.cpp
    Source/UI/SplitterBar.cpp
    Source/Config/ConfigManager.cpp
    Source/Audio/AudioEngine.cpp
    Source/Audio/DspChain.cpp
    Source/Audio/PitchFormant.cpp
    Source/Audio/NoiseGate.cpp
    Source/Soundboard/Soundboard.cpp
    Source/Hotkeys/GlobalHotkeys.cpp
    Source/Presets/PresetManager.cpp
    Source/Tests/SelfTest.cpp)

target_include_directories(VoicePad PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/third_party)

target_compile_definitions(VoicePad PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:VoicePad,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:VoicePad,JUCE_VERSION>")

target_link_libraries(VoicePad
    PRIVATE
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags)
