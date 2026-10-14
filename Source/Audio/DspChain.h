#pragma once

#include <array>
#include <vector>
#include "Common.h"
#include "VoiceParams.h"
#include "PitchFormant.h"
#include "NoiseGate.h"

namespace vp
{
    // Full real-time voice-processing chain (all mouse-controllable, parameter automatable).
    class DspChain
    {
    public:
        explicit DspChain (VoiceParams& paramsToUse);

        void prepare (double sampleRate, int blockSize, int channels);
        void reset();
        void process (juce::AudioBuffer<float>& buffer);

    private:
        void applyHighpass (juce::AudioBuffer<float>& buffer, float freq);
        void applyGate (juce::AudioBuffer<float>& buffer, float thresholdDb);
        void applyDrive (juce::AudioBuffer<float>& buffer, float amount);
        void applyRobot (juce::AudioBuffer<float>& buffer, float mix, float freq);
        void applyEcho (juce::AudioBuffer<float>& buffer, float mix, float timeMs, float feedback);

        VoiceParams& params;

        double sr = 48000.0;
        int channels = 2;
        bool prepared = false;

        std::array<juce::dsp::IIR::Filter<float>, 2> highpassFilters;
        NoiseGate denoiser;
        PitchFormant pitchShifter;
        juce::dsp::Chorus<float> chorus;
        juce::Reverb reverb;
        juce::dsp::Compressor<float> compressor;
        juce::dsp::Limiter<float> limiter;
        juce::dsp::Gain<float> outputGain;

        // echo
        std::vector<float> echoBuffer;
        int echoSize = 0;
        int echoWritePos = 0;

        // robot
        float robotPhase = 0.0f;

        // gate state
        std::array<float, 2> gateEnv   { 0.0f, 0.0f };
        std::array<float, 2> gateGain  { 1.0f, 1.0f };

        float appliedHighpass = -1.0f;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspChain)
    };
}
