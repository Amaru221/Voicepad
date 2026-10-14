#pragma once

#include "Panel.h"
#include "Presets/PresetManager.h"
#include "EventLog.h"

namespace vp
{
    // Preset selector grid + selected-preset detail card (weights, description, timeline).
    class PresetPanel : public Panel, private juce::Timer
    {
    public:
        PresetPanel (PresetManager& presetsToUse, VoiceParams& paramsToUse, EventLog& logToUse);

        void resized() override;
        void paintPanelContent (juce::Graphics&) override;

        void selectPreset (int index, bool notify);
        void highlightOnly (int index);

        std::function<void (int)> onPresetApplied;

    private:
        void timerCallback() override;

        PresetManager& presets;
        VoiceParams& params;
        EventLog& log;

        juce::OwnedArray<juce::TextButton> pads;
        int selected = 0;
    };
}
