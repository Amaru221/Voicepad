#include "SelfTest.h"

#include "Common.h"
#include "Audio/AudioEngine.h"
#include "Audio/DspChain.h"
#include "Audio/VoiceParams.h"
#include "Presets/PresetManager.h"
#include "Soundboard/Soundboard.h"

#include <cmath>
#include <cstdio>
#include <vector>

namespace vp
{
namespace
{
    struct Report
    {
        juce::StringArray lines;
        int passed = 0;
        int failed = 0;

        void check (bool cond, const juce::String& msg)
        {
            lines.add (juce::String (cond ? "[PASS] " : "[FAIL] ") + msg);
            if (cond) ++passed; else ++failed;
        }

        void info (const juce::String& msg) { lines.add ("[INFO] " + msg); }
    };

    float peakOf (const std::vector<float>& v)
    {
        float p = 0.0f;
        for (auto s : v) p = juce::jmax (p, std::abs (s));
        return p;
    }

    double rmsOf (const std::vector<float>& v)
    {
        if (v.empty()) return 0.0;
        double e = 0.0;
        for (auto s : v) e += (double) s * (double) s;
        return std::sqrt (e / (double) v.size());
    }

    double rmsFrom (const std::vector<float>& v, size_t start)
    {
        if (start >= v.size()) return 0.0;
        double e = 0.0;
        for (size_t i = start; i < v.size(); ++i) e += (double) v[i] * (double) v[i];
        return std::sqrt (e / (double) (v.size() - start));
    }

    double tailRms (const std::vector<float>& v, double fraction)
    {
        const size_t start = (size_t) ((double) v.size() * (1.0 - fraction));
        return rmsFrom (v, start);
    }

    std::vector<float> renderClip (Soundboard& board, int slot, float gainDb, int block)
    {
        board.stopAll();
        board.trigger (slot, 1.0f);

        juce::AudioBuffer<float> out (2, block);
        std::vector<float> mono;
        int guard = 0;

        while (board.isPlaying (slot) && guard < 20000)
        {
            out.clear();
            board.renderInto (out, gainDb);
            for (int s = 0; s < block; ++s)
                mono.push_back (out.getSample (0, s));
            ++guard;
        }
        return mono;
    }

    // Deterministic synthetic "voice" (three harmonics, ~ -20 dBFS).
    struct VoiceGen
    {
        double ph[3] { 0.0, 0.0, 0.0 };
        double sr;
        float amp;

        explicit VoiceGen (double sampleRate, float amplitude = 0.15f)
            : sr (sampleRate), amp (amplitude) {}

        float next()
        {
            const double freqs[3] = { 150.0, 300.0, 450.0 };
            const double gains[3] = { 1.0, 0.4, 0.2 };
            double v = 0.0;
            for (int i = 0; i < 3; ++i)
            {
                v += gains[i] * std::sin (ph[i]);
                ph[i] += juce::MathConstants<double>::twoPi * freqs[i] / sr;
                if (ph[i] > juce::MathConstants<double>::twoPi)
                    ph[i] -= juce::MathConstants<double>::twoPi;
            }
            return (float) (v * amp);
        }
    };

    std::vector<float> renderChain (DspChain& chain, int blocks, int block, double sr,
                                    int skipBlocks)
    {
        VoiceGen gen (sr);
        juce::AudioBuffer<float> buf (2, block);
        std::vector<float> out;
        out.reserve ((size_t) blocks * (size_t) block);

        for (int b = 0; b < blocks; ++b)
        {
            for (int s = 0; s < block; ++s)
            {
                const float v = gen.next();
                buf.setSample (0, s, v);
                buf.setSample (1, s, v);
            }

            chain.process (buf);

            if (b >= skipBlocks)
                for (int s = 0; s < block; ++s)
                    out.push_back (buf.getSample (0, s));
        }
        return out;
    }
}

int runSelfTests (const juce::File& outputFile)
{
    const double sr = 48000.0;
    const int block = 512;

    Report r;
    r.info ("VoicePad audio self-test  sr=" + juce::String (sr, 0) + " block=" + juce::String (block));

    // =========================================================================
    // 1. Soundboard: every default clip must be audible and play in full.
    // =========================================================================
    r.info ("--- soundboard clips ---");
    {
        Soundboard board;
        board.prepare (sr, block);
        board.buildDefaultKit();

        const int n = board.getNumSlots();
        r.check (n >= 8, "default kit has >= 8 slots (got " + juce::String (n) + ")");

        for (int i = 0; i < n; ++i)
        {
            const auto mono = renderClip (board, i, 0.0f, block);
            const float peak = peakOf (mono);
            const double rms = rmsOf (mono);
            const double durMs = (double) mono.size() * 1000.0 / sr;
            const double tail = tailRms (mono, 0.2);

            r.check (peak > 0.05f,
                     "clip '" + board.getSlotName (i) + "' audible  peak=" + juce::String (peak, 4)
                         + " rms=" + juce::String (rms, 4));
            r.check (mono.size() > (size_t) (sr * 0.05),
                     "clip '" + board.getSlotName (i) + "' full length  dur=" + juce::String (durMs, 1) + " ms");
            r.check (tail > 0.0005,
                     "clip '" + board.getSlotName (i) + "' energy across time (tail rms=" + juce::String (tail, 5) + ")");
        }

        // Gain response: the -3 dB default must not silence anything.
        r.info ("--- soundboard gain ---");
        for (int db : { 0, -3, -12, 12 })
        {
            Soundboard b2;
            b2.prepare (sr, block);
            b2.buildDefaultKit();
            const auto mono = renderClip (b2, 0, (float) db, block);
            const float peak = peakOf (mono);
            r.check (peak > 0.02f, "clip 0 at " + juce::String (db) + " dB audible (peak="
                                       + juce::String (peak, 4) + ")");
        }
    }

    // =========================================================================
    // 2. Voice presets: every preset must produce an audible, changed signal.
    // =========================================================================
    r.info ("--- voice presets ---");
    {
        PresetManager pm;
        const int np = pm.getNumPresets();
        std::vector<double> presetRms;

        for (int p = 0; p < np; ++p)
        {
            VoiceParams params;
            pm.applyPreset (p, params);
            params.noiseCancel.store (false);

            DspChain chain (params);
            chain.prepare (sr, block, 2);

            const auto out = renderChain (chain, 400, block, sr, 100);
            const double rms = rmsOf (out);
            presetRms.push_back (rms);

            r.check (rms > 0.005,
                     "preset " + pm.getDescriptor (p).name + " output audible (rms="
                         + juce::String (rms, 4) + " peak=" + juce::String (peakOf (out), 4) + ")");
        }

        // A pitch/formant preset must not collapse to (nearly) the dry level.
        if (np > 1)
        {
            const double normal = presetRms[0];
            const double female = presetRms[1];
            r.check (normal > 0.005, "NORMAL preset audible");
            r.check (female > normal * 0.1, "FEMALE preset not collapsed (normal="
                                                + juce::String (normal, 4) + " female=" + juce::String (female, 4) + ")");
        }
    }

    // =========================================================================
    // 2b. Sample-rate change: the kit must be regenerated, not played back
    //     pitched/lengthened.
    // =========================================================================
    r.info ("--- sample-rate handling ---");
    {
        Soundboard b;
        b.prepare (48000.0, block);
        b.buildDefaultKit();

        const auto at48 = renderClip (b, 0, 0.0f, block);   // LASER = 0.35 s
        const double dur48 = (double) at48.size() / 48000.0;

        b.prepare (44100.0, block);                          // device switches
        const auto at44 = renderClip (b, 0, 0.0f, block);
        const double dur44 = (double) at44.size() / 44100.0;

        r.check (std::abs (dur48 - 0.35) < 0.03,
                 "LASER duration at 48 kHz = " + juce::String (dur48, 3) + " s");
        r.check (std::abs (dur44 - 0.35) < 0.03,
                 "LASER duration at 44.1 kHz after rebuild = " + juce::String (dur44, 3) + " s");
        r.check (std::abs (dur48 - dur44) < 0.02,
                 "sample-rate change keeps clip length (" + juce::String (dur48, 3) + " vs "
                     + juce::String (dur44, 3) + " s)");
    }

    // =========================================================================
    // 2c. Pitch tracking: presets must actually shift the fundamental.
    // =========================================================================
    r.info ("--- pitch tracking ---");
    {
        auto estimateFreq = [&] (const std::vector<float>& v) -> double
        {
            if (v.size() < 16) return 0.0;
            int crossings = 0;
            for (size_t i = 1; i < v.size(); ++i)
                if ((v[i - 1] < 0.0f) != (v[i] < 0.0f))
                    ++crossings;
            return (double) crossings * sr / (2.0 * (double) v.size());
        };

        auto toneFreq = [&] (float pitchSemis) -> double
        {
            VoiceParams params;
            params.pitchSemis.store (pitchSemis);
            DspChain chain (params);
            chain.prepare (sr, block, 2);

            std::vector<float> out;
            const int blocks = 400;
            for (int b = 0; b < blocks; ++b)
            {
                juce::AudioBuffer<float> buf (2, block);
                for (int s = 0; s < block; ++s)
                {
                    const float v = 0.15f * (float) std::sin (juce::MathConstants<double>::twoPi * 150.0
                                                               * (double) (b * block + s) / sr);
                    buf.setSample (0, s, v);
                    buf.setSample (1, s, v);
                }
                chain.process (buf);
                if (b >= 150)
                    for (int s = 0; s < block; ++s)
                        out.push_back (buf.getSample (0, s));
            }
            return estimateFreq (out);
        };

        const double dry  = toneFreq (0.0f);    // 150 Hz
        const double up12 = toneFreq (12.0f);   // ~300 Hz
        const double dn6  = toneFreq (-6.0f);   // ~106 Hz

        r.check (std::abs (dry - 150.0) < 25.0, "dry fundamental ~150 Hz (got " + juce::String (dry, 1) + ")");
        r.check (std::abs (up12 - 300.0) < 45.0, "+12 st fundamental ~300 Hz (got " + juce::String (up12, 1) + ")");
        r.check (std::abs (dn6 - 106.0) < 25.0, "-6 st fundamental ~106 Hz (got " + juce::String (dn6, 1) + ")");
    }

    // =========================================================================
    // 3. Noise reduction: must pass voice, must not gate the soundboard.
    // =========================================================================
    r.info ("--- noise gate / noise cancel ---");
    {
        VoiceParams params;
        DspChain chain (params);
        chain.prepare (sr, block, 2);

        auto steadyRms = [&] (bool nc, float threshold, float amp)
        {
            params.noiseCancel.store (nc);
            params.gateThreshold.store (threshold);
            params.noiseThresholdDb.store (threshold);
            chain.reset();

            std::vector<float> out;
            for (int b = 0; b < 200; ++b)
            {
                juce::AudioBuffer<float> buf (2, block);
                for (int s = 0; s < block; ++s)
                {
                    const float v = amp * (float) std::sin (juce::MathConstants<double>::twoPi * 220.0
                                                            * (double) (b * block + s) / sr);
                    buf.setSample (0, s, v);
                    buf.setSample (1, s, v);
                }
                chain.process (buf);
                if (b > 50)
                    for (int s = 0; s < block; ++s)
                        out.push_back (buf.getSample (0, s));
            }
            return rmsOf (out);
        };

        const float dryVoice  = steadyRms (false, -45.0f, 0.05f);
        const float ncVoice   = steadyRms (true,  -45.0f, 0.05f);
        const float ncNoise   = steadyRms (true,  -45.0f, 0.0005f);
        const float dryNoise  = steadyRms (false, -45.0f, 0.0005f);

        r.check (dryVoice > 0.01f, "voice passes with noise-cancel OFF (rms=" + juce::String (dryVoice, 4) + ")");
        r.check (ncVoice > 0.01f, "voice passes with noise-cancel ON (rms=" + juce::String (ncVoice, 4) + ")");
        r.check (ncNoise < dryNoise * 0.5f, "low-level noise reduced with noise-cancel ON (dry="
                                                + juce::String (dryNoise, 5) + " nc=" + juce::String (ncNoise, 5) + ")");

        // The soundboard is mixed AFTER the chain: a closed gate on a silent mic
        // must not remove the board output.
        VoiceParams mixParams;
        mixParams.noiseCancel.store (true);
        mixParams.gateThreshold.store (-30.0f);
        mixParams.noiseThresholdDb.store (-30.0f);
        mixParams.soundboardGainDb.store (0.0f);

        DspChain mixChain (mixParams);
        mixChain.prepare (sr, block, 2);
        Soundboard board;
        board.prepare (sr, block);
        board.buildDefaultKit();
        board.stopAll();
        board.trigger (0, 1.0f);

        double boardEnergy = 0.0;
        juce::AudioBuffer<float> mixBuf (2, block);
        for (int b = 0; b < 40; ++b)
        {
            mixBuf.clear();               // silent microphone
            mixChain.process (mixBuf);    // gate fully closed
            board.renderInto (mixBuf, mixParams.soundboardGainDb.load());
            for (int s = 0; s < block; ++s)
            {
                const float x = mixBuf.getSample (0, s);
                boardEnergy += (double) x * x;
            }
        }
        r.check (boardEnergy > 0.01, "soundboard audible while gate is closed (energy="
                                         + juce::String (boardEnergy, 4) + ")");
    }

    // =========================================================================
    // 3b. Engine buffering: a clip must play in full even when the device block
    //     size varies (the internal store can be larger than the current block).
    // =========================================================================
    r.info ("--- engine buffering (variable block size) ---");
    {
        VoiceParams params;
        DspChain chain (params);
        Soundboard board;
        AudioEngine engine (params, chain, board);
        chain.prepare (sr, block, 2);
        board.prepare (sr, block);
        board.buildDefaultKit();

        auto runBlock = [&] (int n, bool capture, long& rendered)
        {
            std::vector<float> inL ((size_t) n, 0.0f), inR ((size_t) n, 0.0f);
            const float* in[2] = { inL.data(), inR.data() };
            std::vector<float> outL ((size_t) n, 0.0f), outR ((size_t) n, 0.0f);
            float* out[2] = { outL.data(), outR.data() };
            juce::AudioIODeviceCallbackContext ctx {};
            engine.audioDeviceIOCallbackWithContext (in, 2, out, 2, n, ctx);
            if (capture)
                rendered += n;
        };

        long warm = 0;
        for (int i = 0; i < 4; ++i)
            runBlock (1024, false, warm);        // grow the internal store to 1024

        board.stopAll();
        board.trigger (0, 1.0f);                 // LASER = 352 ms
        long rendered = 0;
        for (int i = 0; i < 4000 && board.isPlaying (0); ++i)
            runBlock (256, true, rendered);      // now render in smaller blocks

        const double durMs = (double) rendered * 1000.0 / sr;
        r.check (std::abs (durMs - 352.0) < 25.0,
                 "soundboard plays full length through engine (1024 then 256 blocks), dur="
                     + juce::String (durMs, 1) + " ms");
    }

    // =========================================================================
    // 4. Real-time factor per preset (CPU headroom).
    // =========================================================================
    r.info ("--- real-time factor ---");
    {
        PresetManager pm;
        for (int p = 0; p < pm.getNumPresets(); ++p)
        {
            VoiceParams params;
            pm.applyPreset (p, params);
            DspChain chain (params);
            chain.prepare (sr, block, 2);

            const int blocks = 200;
            const double audioSeconds = (double) blocks * (double) block / sr;

            VoiceGen gen (sr);
            juce::AudioBuffer<float> buf (2, block);

            const double t0 = juce::Time::getMillisecondCounterHiRes();
            for (int b = 0; b < blocks; ++b)
            {
                for (int s = 0; s < block; ++s)
                {
                    const float v = gen.next();
                    buf.setSample (0, s, v);
                    buf.setSample (1, s, v);
                }
                chain.process (buf);
            }
            const double elapsed = (juce::Time::getMillisecondCounterHiRes() - t0) / 1000.0;
            const double rtf = elapsed / audioSeconds;

            r.check (rtf < 0.8,
                     "preset " + pm.getDescriptor (p).name + " real-time factor="
                         + juce::String (rtf, 3) + " (" + juce::String (rtf * 100.0, 1) + "% of one core)");
        }
    }

    // =========================================================================
    // Report
    // =========================================================================
    r.lines.add ("");
    r.lines.add ("RESULT: " + juce::String (r.passed) + " passed, " + juce::String (r.failed) + " failed");

    const auto text = r.lines.joinIntoString ("\n");
    outputFile.getParentDirectory().createDirectory();
    outputFile.replaceWithText (text);

    std::printf ("%s\n", text.toRawUTF8());
    std::fflush (stdout);

    return r.failed;
}
}
