#pragma once

#include <atomic>
#include <vector>
#include <juce_audio_basics/juce_audio_basics.h>

// Signalsmith Stretch :: real-time pitch/formant shifting.
// Source: https://github.com/Signalsmith-Audio/signalsmith-stretch
// Licensed free for personal / non-commercial use.
#include "signalsmith-stretch.h"

namespace vp
{
    class PitchFormant
    {
    public:
        PitchFormant() = default;

        void prepare (double sampleRate, int maxBlock, int channels);
        void reset();

        void setTargets (float pitchSemitones, float formantSemitones);

        // In-place processing of an N-channel buffer (N == channels used in prepare).
        void process (juce::AudioBuffer<float>& buffer);

        bool isEngaged() const noexcept;
        int  latencySamples() const;

    private:
        signalsmith::stretch::SignalsmithStretch<float> stretch;
        std::vector<const float*> inPtrs;
        std::vector<float*>       outPtrs;
        std::vector<float>        scratch;
        juce::AudioBuffer<float>  outputScratch;

        double sampleRate = 48000.0;
        int    numChannels = 2;
        int    maxBlockSize = 512;
        bool   prepared = false;

        std::atomic<float> targetPitch   { 0.0f };
        std::atomic<float> targetFormant { 0.0f };

        float appliedPitch = -999.0f;
        float appliedFormant = -999.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PitchFormant)
    };
}
