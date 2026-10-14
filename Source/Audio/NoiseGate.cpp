#include "NoiseGate.h"

namespace vp
{
    void NoiseGate::prepare (double sampleRate, int maxBlock, int channelsToUse)
    {
        sr = sampleRate > 0 ? sampleRate : 48000.0;
        numChannels = juce::jmax (1, channelsToUse);
        prepared = true;
        juce::ignoreUnused (maxBlock);

        // Envelope detector (fast up, slow down).
        detAttack   = (float) std::exp (-1.0 / (0.002 * sr));
        detRelease  = (float) std::exp (-1.0 / (0.120 * sr));

        // Gain smoother (fast open, slow close).
        gainAttack  = (float) std::exp (-1.0 / (0.005 * sr));
        gainRelease = (float) std::exp (-1.0 / (0.180 * sr));

        env  = { 0.0f, 0.0f };
        gain = { 1.0f, 1.0f };
    }

    void NoiseGate::reset()
    {
        env  = { 0.0f, 0.0f };
        gain = { 1.0f, 1.0f };
    }

    void NoiseGate::process (juce::AudioBuffer<float>& buffer, bool enabled, float thresholdDb)
    {
        if (! prepared || ! enabled)
            return;

        const float expansion = 6.0f;    // dB reduction slope below threshold
        const float floorDb = -60.0f;    // maximum attenuation
        const float hysteresis = 5.0f;   // dB

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (numChannels, buffer.getNumChannels());

        for (int c = 0; c < ch; ++c)
        {
            auto* d = buffer.getWritePointer (c);
            float e = env[(size_t) c];
            float g = gain[(size_t) c];

            for (int i = 0; i < n; ++i)
            {
                const float x = d[i];
                const float rect = std::abs (x);

                e = (rect > e) ? rect + detAttack * (e - rect)
                               : rect + detRelease * (e - rect);

                const float eDb = juce::Decibels::gainToDecibels (e, -100.0f);

                // Hysteresis: harder to open than to stay open.
                const bool isOpen = g > 0.5f;
                const float openThreshold = isOpen ? (thresholdDb - hysteresis) : thresholdDb;

                float gTarget;
                if (eDb >= openThreshold)
                {
                    gTarget = 1.0f;
                }
                else
                {
                    float gDb = (eDb - thresholdDb) * (expansion - 1.0f);
                    gDb = juce::jlimit (floorDb, 0.0f, gDb);
                    gTarget = juce::Decibels::decibelsToGain (gDb);
                }

                g = (gTarget > g) ? gTarget + gainAttack * (g - gTarget)
                                  : gTarget + gainRelease * (g - gTarget);

                d[i] = x * g;
            }

            env[(size_t) c] = e;
            gain[(size_t) c] = g;
        }
    }
}
