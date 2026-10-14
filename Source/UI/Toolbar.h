#pragma once

#include "Common.h"
#include "Audio/AudioEngine.h"

namespace vp
{
    // Top command strip: identity, live status lamps, epoch counter, theme picker.
    class Toolbar : public juce::Component, private juce::Timer
    {
    public:
        Toolbar (AudioEngine& engineToUse, VoiceParams& paramsToUse);

        void paint (juce::Graphics&) override;
        void resized() override;

        std::function<void()> onPowerToggled;
        std::function<void (int)> onThemeChanged;

        void setThemeIndex (int index);

    private:
        void timerCallback() override;
        void drawLamp (juce::Graphics&, juce::Rectangle<int>, const juce::String& label, bool on, juce::Colour colour);

        AudioEngine& engine;
        VoiceParams& params;
        juce::TextButton powerButton { "PROCESS: ON" };
        juce::Label themeLabel;
        juce::ComboBox themeBox;
        double epochStart = 0.0;
    };
}
