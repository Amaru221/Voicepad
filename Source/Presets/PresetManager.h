#pragma once

#include "Common.h"
#include "Audio/VoiceParams.h"

namespace vp
{
    struct PresetDescriptor
    {
        int index = 0;
        juce::String name;
        juce::String tag;
        juce::String description;
    };

    class PresetManager
    {
    public:
        PresetManager();

        int getNumPresets() const noexcept { return (int) presets.size(); }
        const Preset& getPreset (int index) const;
        PresetDescriptor getDescriptor (int index) const;

        void applyPreset (int index, VoiceParams& params) const;

        int  getSelected() const noexcept { return selected; }
        void setSelected (int index);

    private:
        std::vector<Preset> presets;
        std::vector<PresetDescriptor> descriptors;
        int selected = 0;
    };
}
