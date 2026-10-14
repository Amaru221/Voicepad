#pragma once

#include <atomic>
#include <cmath>
#include <juce_core/juce_core.h>

namespace vp
{
    // Real-time voice parameters shared between the UI (message thread) and the audio engine.
    struct VoiceParams
    {
        std::atomic<float> inputGainDb   { 0.0f };
        std::atomic<float> gateThreshold { -55.0f };
        std::atomic<float> highpassHz    { 90.0f };

        std::atomic<float> pitchSemis    { 0.0f };
        std::atomic<float> formantSemis  { 0.0f };

        std::atomic<float> drive         { 0.0f };
        std::atomic<float> robotMix      { 0.0f };
        std::atomic<float> robotFreq     { 55.0f };

        std::atomic<float> chorusMix     { 0.0f };
        std::atomic<float> chorusRate    { 1.2f };
        std::atomic<float> chorusDepth   { 0.35f };

        std::atomic<float> echoMix       { 0.0f };
        std::atomic<float> echoTimeMs    { 240.0f };
        std::atomic<float> echoFeedback  { 0.32f };

        std::atomic<float> reverbMix     { 0.0f };
        std::atomic<float> compAmount    { 0.25f };
        std::atomic<float> outputGainDb  { 0.0f };
        std::atomic<float> soundboardGainDb { 0.0f };

        std::atomic<bool>  noiseCancel   { false };
        std::atomic<float> noiseThresholdDb { -45.0f };

        std::atomic<bool>  bypass        { false };

        void copyValuesFrom (const VoiceParams& other)
        {
            inputGainDb.store (other.inputGainDb.load());
            gateThreshold.store (other.gateThreshold.load());
            highpassHz.store (other.highpassHz.load());
            pitchSemis.store (other.pitchSemis.load());
            formantSemis.store (other.formantSemis.load());
            drive.store (other.drive.load());
            robotMix.store (other.robotMix.load());
            robotFreq.store (other.robotFreq.load());
            chorusMix.store (other.chorusMix.load());
            chorusRate.store (other.chorusRate.load());
            chorusDepth.store (other.chorusDepth.load());
            echoMix.store (other.echoMix.load());
            echoTimeMs.store (other.echoTimeMs.load());
            echoFeedback.store (other.echoFeedback.load());
            reverbMix.store (other.reverbMix.load());
            compAmount.store (other.compAmount.load());
            outputGainDb.store (other.outputGainDb.load());
            soundboardGainDb.store (other.soundboardGainDb.load());
            noiseCancel.store (other.noiseCancel.load());
            noiseThresholdDb.store (other.noiseThresholdDb.load());
        }
    };

    // A named voice preset (plain values, applied onto live VoiceParams).
    struct Preset
    {
        juce::String name;
        juce::String tag;
        float pitchSemis   = 0.0f;
        float formantSemis = 0.0f;
        float drive        = 0.0f;
        float robotMix     = 0.0f;
        float robotFreq    = 55.0f;
        float chorusMix    = 0.0f;
        float chorusRate   = 1.2f;
        float chorusDepth  = 0.35f;
        float echoMix      = 0.0f;
        float echoTimeMs   = 240.0f;
        float echoFeedback = 0.32f;
        float reverbMix    = 0.0f;
        float compAmount   = 0.25f;
        float outputGainDb = 0.0f;
        float gateThreshold = -55.0f;
        float highpassHz   = 90.0f;

        void applyTo (VoiceParams& p) const
        {
            p.pitchSemis.store (pitchSemis);
            p.formantSemis.store (formantSemis);
            p.drive.store (drive);
            p.robotMix.store (robotMix);
            p.robotFreq.store (robotFreq);
            p.chorusMix.store (chorusMix);
            p.chorusRate.store (chorusRate);
            p.chorusDepth.store (chorusDepth);
            p.echoMix.store (echoMix);
            p.echoTimeMs.store (echoTimeMs);
            p.echoFeedback.store (echoFeedback);
            p.reverbMix.store (reverbMix);
            p.compAmount.store (compAmount);
            p.outputGainDb.store (outputGainDb);
            p.gateThreshold.store (gateThreshold);
            p.highpassHz.store (highpassHz);
        }
    };
}
