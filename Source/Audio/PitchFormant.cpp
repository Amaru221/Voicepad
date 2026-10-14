#include "PitchFormant.h"

namespace vp
{
    void PitchFormant::prepare (double sr, int maxBlock, int channels)
    {
        sampleRate = sr;
        numChannels = juce::jmax (1, channels);
        maxBlockSize = juce::jmax (64, maxBlock);
        prepared = true;

        const int blockSamples = juce::jmax (1024, (int) (sampleRate * 0.045));
        const int intervalSamples = juce::jmax (256, (int) (sampleRate * 0.011));

        stretch.configure (numChannels, blockSamples, intervalSamples);

        inPtrs.assign ((size_t) numChannels, nullptr);
        outPtrs.assign ((size_t) numChannels, nullptr);
        scratch.assign ((size_t) maxBlockSize, 0.0f);
        outputScratch.setSize (numChannels, maxBlockSize, false, true, true);

        appliedPitch = -999.0f;
        appliedFormant = -999.0f;
    }

    void PitchFormant::reset()
    {
        if (prepared)
            stretch.reset();
        appliedPitch = -999.0f;
        appliedFormant = -999.0f;
    }

    void PitchFormant::setTargets (float pitchSemitones, float formantSemitones)
    {
        targetPitch.store (pitchSemitones);
        targetFormant.store (formantSemitones);
    }

    bool PitchFormant::isEngaged() const noexcept
    {
        return std::abs (targetPitch.load())   > 0.01f
            || std::abs (targetFormant.load()) > 0.01f;
    }

    int PitchFormant::latencySamples() const
    {
        if (! prepared)
            return 0;
        return stretch.inputLatency() + stretch.outputLatency();
    }

    void PitchFormant::process (juce::AudioBuffer<float>& buffer)
    {
        if (! prepared)
            return;

        const float pitch = targetPitch.load();
        const float formant = targetFormant.load();

        if (std::abs (pitch) <= 0.01f && std::abs (formant) <= 0.01f)
            return; // fully dry, zero added latency

        if (pitch != appliedPitch || formant != appliedFormant)
        {
            stretch.setTransposeSemitones (pitch);
            stretch.setFormantSemitones (formant, true);
            appliedPitch = pitch;
            appliedFormant = formant;
        }

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (numChannels, buffer.getNumChannels());

        // Signalsmith Stretch does not support aliasing between input and output,
        // so render into a separate buffer and copy the result back.
        if (outputScratch.getNumSamples() < n)
            outputScratch.setSize (numChannels, n, false, false, true);

        for (int c = 0; c < ch; ++c)
        {
            inPtrs[(size_t) c]  = buffer.getReadPointer (c);
            outPtrs[(size_t) c] = outputScratch.getWritePointer (c);
        }

        const float* const* in = inPtrs.data();
        float* const* out = outPtrs.data();

        stretch.process (in, n, out, n);

        for (int c = 0; c < ch; ++c)
            buffer.copyFrom (c, 0, outputScratch, c, 0, n);
    }
}
