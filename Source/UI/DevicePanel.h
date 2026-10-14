#pragma once

#include "Panel.h"
#include "Audio/AudioEngine.h"

namespace vp
{
    // Device routing panel: microphone source and virtual-cable destination.
    class DevicePanel : public Panel
    {
    public:
        explicit DevicePanel (AudioEngine& engineToUse);

        void resized() override;
        void paintPanelContent (juce::Graphics&) override;

        void refreshLists();

        std::function<void (const juce::String&)> onLog;

    private:
        void applySelection();
        void updateStartButton();

        AudioEngine& engine;
        juce::Label inLabel, outLabel;
        juce::ComboBox inputBox, outputBox;
        juce::TextButton autoButton { "AUTO-ROUTE" };
        juce::TextButton refreshButton { "RESCAN" };
        juce::TextButton startButton { "ENGAGE" };
    };
}
