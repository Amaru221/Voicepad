#include "DspChain.h"

namespace vp
{
    DspChain::DspChain (VoiceParams& paramsToUse) : params (paramsToUse)
    {
    }

    void DspChain::prepare (double sampleRate, int blockSize, int numChannels)
    {
        sr = sampleRate;
        channels = juce::jmax (1, numChannels);
        prepared = true;

        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) juce::jmax (16, blockSize), (juce::uint32) channels };

        for (auto& f : highpassFilters)
        {
            f.prepare (spec);
            f.reset();
        }
        appliedHighpass = -1.0f;
        pitchShifter.prepare (sampleRate, blockSize, channels);
        denoiser.prepare (sampleRate, blockSize, channels);

        chorus.prepare (spec);
        chorus.reset();
        chorus.setCentreDelay (18.0f);

        reverb.setSampleRate (sampleRate);
        reverb.reset();

        compressor.prepare (spec);
        compressor.reset();

        limiter.prepare (spec);
        limiter.reset();

        outputGain.prepare (spec);
        outputGain.reset();

        echoSize = (int) (sampleRate * 0.8) + 1;
        echoBuffer.assign ((size_t) echoSize * (size_t) channels, 0.0f);
        echoWritePos = 0;
        robotPhase = 0.0f;
        gateEnv = { 0.0f, 0.0f };
        gateGain = { 1.0f, 1.0f };
    }

    void DspChain::reset()
    {
        if (! prepared)
            return;

        for (auto& f : highpassFilters)
            f.reset();
        denoiser.reset();
        pitchShifter.reset();
        chorus.reset();
        reverb.reset();
        compressor.reset();
        limiter.reset();
        outputGain.reset();

        std::fill (echoBuffer.begin(), echoBuffer.end(), 0.0f);
        echoWritePos = 0;
        robotPhase = 0.0f;
        gateEnv = { 0.0f, 0.0f };
        gateGain = { 1.0f, 1.0f };
    }

    void DspChain::applyHighpass (juce::AudioBuffer<float>& buffer, float freq)
    {
        auto coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass (sr, freq);

        if (std::abs (freq - appliedHighpass) > 0.5f)
        {
            for (auto& f : highpassFilters)
                f.coefficients = coeffs;
            appliedHighpass = freq;
        }

        const int ch = juce::jmin ((int) highpassFilters.size(), buffer.getNumChannels());
        juce::dsp::AudioBlock<float> block (buffer);
        for (int c = 0; c < ch; ++c)
        {
            auto channelBlock = block.getSingleChannelBlock ((size_t) c);
            juce::dsp::ProcessContextReplacing<float> ctx (channelBlock);
            highpassFilters[(size_t) c].process (ctx);
        }
    }

    void DspChain::applyGate (juce::AudioBuffer<float>& buffer, float thresholdDb)
    {
        const float threshold = juce::Decibels::decibelsToGain (thresholdDb);
        const float atk = (float) std::exp (-1.0 / (0.002 * sr));
        const float rel = (float) std::exp (-1.0 / (0.060 * sr));
        const float gainSmooth = (float) std::exp (-1.0 / (0.005 * sr));

        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (channels, buffer.getNumChannels());

        for (int c = 0; c < ch; ++c)
        {
            auto* d = buffer.getWritePointer (c);
            float env = gateEnv[(size_t) c];
            float g = gateGain[(size_t) c];

            for (int i = 0; i < n; ++i)
            {
                const float x = std::abs (d[i]);
                env = (x > env) ? (x + atk * (env - x)) : (x + rel * (env - x));

                const float target = (env > threshold) ? 1.0f : 0.0f;
                g = target + gainSmooth * (g - target);

                d[i] *= g;
            }

            gateEnv[(size_t) c] = env;
            gateGain[(size_t) c] = g;
        }
    }

    void DspChain::applyDrive (juce::AudioBuffer<float>& buffer, float amount)
    {
        if (amount <= 0.001f)
            return;

        const float drive = 1.0f + amount * 12.0f;
        const float comp = 1.0f / (float) std::tanh (drive);
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (channels, buffer.getNumChannels());

        for (int c = 0; c < ch; ++c)
        {
            auto* d = buffer.getWritePointer (c);
            for (int i = 0; i < n; ++i)
                d[i] = std::tanh (d[i] * drive) * comp;
        }
    }

    void DspChain::applyRobot (juce::AudioBuffer<float>& buffer, float mix, float freq)
    {
        if (mix <= 0.001f)
            return;

        const float inc = juce::MathConstants<float>::twoPi * freq / (float) sr;
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (channels, buffer.getNumChannels());

        for (int i = 0; i < n; ++i)
        {
            const float mod = std::sin (robotPhase);
            robotPhase += inc;
            if (robotPhase > juce::MathConstants<float>::twoPi)
                robotPhase -= juce::MathConstants<float>::twoPi;

            for (int c = 0; c < ch; ++c)
            {
                auto* d = buffer.getWritePointer (c);
                d[i] = d[i] * (1.0f - mix) + (d[i] * mod) * mix;
            }
        }
    }

    void DspChain::applyEcho (juce::AudioBuffer<float>& buffer, float mix, float timeMs, float feedback)
    {
        if (mix <= 0.001f || echoSize <= 0)
            return;

        const int delaySamples = juce::jlimit (1, echoSize - 1, (int) (timeMs * 0.001f * sr));
        const int n = buffer.getNumSamples();
        const int ch = juce::jmin (channels, buffer.getNumChannels());

        for (int i = 0; i < n; ++i)
        {
            int readPos = echoWritePos - delaySamples;
            if (readPos < 0)
                readPos += echoSize;

            for (int c = 0; c < ch; ++c)
            {
                auto* d = buffer.getWritePointer (c);
                auto* line = echoBuffer.data() + (size_t) c * (size_t) echoSize;

                const float delayed = line[readPos];
                line[echoWritePos] = d[i] + delayed * feedback;
                d[i] = d[i] + delayed * mix;
            }

            if (++echoWritePos >= echoSize)
                echoWritePos = 0;
        }
    }

    void DspChain::process (juce::AudioBuffer<float>& buffer)
    {
        if (! prepared)
            return;

        if (params.bypass.load())
            return;

        const float inputGainDb = params.inputGainDb.load();
        if (std::abs (inputGainDb) > 0.01f)
            buffer.applyGain (juce::Decibels::decibelsToGain (inputGainDb));

        // Noise reduction is opt-in: the gate and the expander only run while
        // NOISE CANCEL is on, so the voice path stays fully open (and presets
        // unchopped) when it is off.
        const bool noiseReduction = params.noiseCancel.load();
        if (noiseReduction)
            applyGate (buffer, params.gateThreshold.load());

        applyHighpass (buffer, params.highpassHz.load());

        if (noiseReduction)
            denoiser.process (buffer, true, params.noiseThresholdDb.load());

        pitchShifter.setTargets (params.pitchSemis.load(), params.formantSemis.load());
        pitchShifter.process (buffer);

        applyDrive (buffer, params.drive.load());

        // chorus
        {
            const float mix = params.chorusMix.load();
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            if (mix > 0.001f)
            {
                chorus.setRate (juce::jlimit (0.1f, 8.0f, params.chorusRate.load()));
                chorus.setDepth (juce::jlimit (0.0f, 1.0f, params.chorusDepth.load()));
                chorus.setMix (juce::jlimit (0.0f, 1.0f, mix));
                chorus.process (ctx);
            }
        }

        applyRobot (buffer, params.robotMix.load(), params.robotFreq.load());
        applyEcho (buffer, params.echoMix.load(), params.echoTimeMs.load(), params.echoFeedback.load());

        // reverb (insert, dry preserved)
        {
            const float mix = params.reverbMix.load();
            if (buffer.getNumChannels() >= 2)
            {
                juce::Reverb::Parameters rp {};
                rp.roomSize = 0.6f;
                rp.damping = 0.5f;
                rp.width = 1.0f;
                rp.wetLevel = juce::jlimit (0.0f, 1.0f, mix);
                rp.dryLevel = 1.0f;
                rp.freezeMode = 0.0f;
                reverb.setParameters (rp);
                reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
            }
        }

        // compressor
        {
            const float amount = juce::jlimit (0.0f, 1.0f, params.compAmount.load());
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            compressor.setThreshold (-6.0f - amount * 24.0f);
            compressor.setRatio (1.0f + amount * 7.0f);
            compressor.setAttack (5.0f);
            compressor.setRelease (120.0f);
            compressor.process (ctx);
        }

        // output gain
        {
            const float gainDb = params.outputGainDb.load();
            if (std::abs (gainDb) > 0.01f)
            {
                juce::dsp::AudioBlock<float> block (buffer);
                juce::dsp::ProcessContextReplacing<float> ctx (block);
                outputGain.setGainDecibels (gainDb);
                outputGain.process (ctx);
            }
        }

        // safety limiter
        {
            juce::dsp::AudioBlock<float> block (buffer);
            juce::dsp::ProcessContextReplacing<float> ctx (block);
            limiter.setThreshold (-1.0f);
            limiter.setRelease (80.0f);
            limiter.process (ctx);
        }
    }
}
