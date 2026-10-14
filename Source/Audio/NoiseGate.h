#pragma once

#include <array>
#include "Common.h"

namespace vp
{
    // Noise reducer based on a downward expander / gate operating in the time
    // domain (no STFT, so it cannot add "musical noise" or interference).
    //
    // - Below the dB threshold the signal is attenuated smoothly toward a floor.
    // - Above the threshold the voice passes through untouched.
    // - Hysteresis + smoothed envelope/gain avoid chattering and chopping.
    class NoiseGate
    {
    public:
        NoiseGate() = default;

        void prepare (double sampleRate, int maxBlock, int channels);
        void reset();

        // thresholdDb: level below which noise is suppressed (e.g. -45 dB).
        void process (juce::AudioBuffer<float>& buffer, bool enabled, float thresholdDb);

    private:
        double sr = 48000.0;
        int numChannels = 2;
        bool prepared = false;

        float detAttack = 0.0f, detRelease = 0.0f;
        float gainAttack = 0.0f, gainRelease = 0.0f;

        std::array<float, 2> env  { { 0.0f, 0.0f } };
        std::array<float, 2> gain { { 1.0f, 1.0f } };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseGate)
    };
}
