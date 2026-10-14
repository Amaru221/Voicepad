#pragma once

#include "Panel.h"
#include "Audio/VoiceParams.h"

namespace vp
{
    // Effect parameter bank: horizontal sliders (mouse / touch only).
    class EffectsPanel : public Panel, private juce::Timer
    {
    public:
        explicit EffectsPanel (VoiceParams& paramsToUse);

        void resized() override;

    private:
        void addControl (juce::String name, std::atomic<float>* target,
                         float min, float max, juce::String suffix, int decimals);
        void updateNoiseButton();
        void timerCallback() override;

        VoiceParams& params;

        juce::TextButton noiseButton { "NOISE CANCEL: OFF" };
        juce::OwnedArray<juce::Label> labels;
        juce::OwnedArray<juce::Slider> sliders;
        std::vector<std::atomic<float>*> targets;
        std::vector<int> decimals;
    };
}
