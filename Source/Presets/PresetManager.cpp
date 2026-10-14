#include "PresetManager.h"

namespace vp
{
    PresetManager::PresetManager()
    {
        auto add = [this] (juce::String name, juce::String tag, juce::String desc, Preset p)
        {
            p.name = name;
            p.tag = tag;
            const int idx = (int) presets.size();
            presets.push_back (p);
            descriptors.push_back ({ idx, name, tag, desc });
        };

        {
            Preset p; p.gateThreshold = -60.0f; p.compAmount = 0.2f;
            add ("NORMAL", "VX-01", "Clean passthrough with light compression and noise gate. Use as neutral baseline.",
                 p);
        }
        {
            Preset p; p.pitchSemis = 7.0f; p.formantSemis = 4.0f; p.compAmount = 0.25f;
            add ("FEMALE", "VX-02", "Upward pitch and formant shift for a lighter, higher register.", p);
        }
        {
            Preset p; p.pitchSemis = -6.0f; p.formantSemis = -3.0f; p.compAmount = 0.3f;
            add ("DEEP", "VX-03", "Downward pitch and formant shift for a heavier, deeper tone.", p);
        }
        {
            Preset p; p.robotMix = 0.9f; p.robotFreq = 58.0f; p.drive = 0.15f;
            add ("ROBOT", "VX-04", "Ring-modulated monotone synthesiser voice with light saturation.", p);
        }
        {
            Preset p; p.pitchSemis = -9.0f; p.formantSemis = -4.0f; p.drive = 0.4f; p.reverbMix = 0.22f;
            add ("DEMON", "VX-05", "Heavy low pitch with saturation and a resonant chamber tail.", p);
        }
        {
            Preset p; p.pitchSemis = 12.0f; p.formantSemis = 9.0f; p.compAmount = 0.2f;
            add ("CHIPMUNK", "VX-06", "Extreme upward shift for a small, fast, cartoon register.", p);
        }
        {
            Preset p; p.reverbMix = 0.5f; p.echoMix = 0.3f; p.echoTimeMs = 300.0f; p.echoFeedback = 0.35f;
            add ("CAVE", "VX-07", "Large reverb room plus a long slapback echo for cave acoustics.", p);
        }
        {
            Preset p; p.pitchSemis = 5.0f; p.formantSemis = 6.0f; p.chorusMix = 0.45f; p.chorusRate = 1.6f;
            add ("ANIME", "VX-08", "Bright shift with a wide chorus for an animated character timbre.", p);
        }

        selected = 0;
    }

    const Preset& PresetManager::getPreset (int index) const
    {
        const int i = juce::jlimit (0, (int) presets.size() - 1, index);
        return presets[(size_t) i];
    }

    PresetDescriptor PresetManager::getDescriptor (int index) const
    {
        const int i = juce::jlimit (0, (int) descriptors.size() - 1, index);
        return descriptors[(size_t) i];
    }

    void PresetManager::applyPreset (int index, VoiceParams& params) const
    {
        getPreset (index).applyTo (params);
    }

    void PresetManager::setSelected (int index)
    {
        selected = juce::jlimit (0, (int) presets.size() - 1, index);
    }
}
