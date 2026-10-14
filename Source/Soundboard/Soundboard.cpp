#include "Soundboard.h"

namespace vp
{
    void Soundboard::prepare (double sampleRate, int block)
    {
        const double newRate = sampleRate > 0 ? sampleRate : 48000.0;
        blockSize = juce::jmax (16, block);
        formatManager.registerBasicFormats();

        if (! prepared || std::abs (newRate - sr) < 0.5)
        {
            sr = newRate;
            prepared = true;
            return;
        }

        // Device rate changed: regenerate the built-in kit and re-resample user
        // clips so their pitch/length stay correct.
        rebuildClips (newRate);
        prepared = true;
    }

    void Soundboard::resampleTo (juce::AudioBuffer<float>& buffer, double fromRate, double toRate)
    {
        if (fromRate <= 0.0 || toRate <= 0.0 || std::abs (fromRate - toRate) < 1.0)
            return;

        const double ratio = toRate / fromRate;
        const int newLen = juce::jmax (1, (int) (buffer.getNumSamples() * ratio));
        juce::AudioBuffer<float> resampled (juce::jmax (1, buffer.getNumChannels()), newLen);

        for (int c = 0; c < buffer.getNumChannels(); ++c)
        {
            auto* src = buffer.getReadPointer (c);
            auto* dst = resampled.getWritePointer (juce::jmin (c, resampled.getNumChannels() - 1));
            for (int i = 0; i < newLen; ++i)
            {
                const double pos = i / ratio;
                const int i0 = (int) pos;
                const int i1 = juce::jmin (i0 + 1, buffer.getNumSamples() - 1);
                const float frac = (float) (pos - i0);
                dst[i] = src[i0] * (1.0f - frac) + src[i1] * frac;
            }
        }

        buffer = std::move (resampled);
    }

    void Soundboard::downmixToMono (juce::AudioBuffer<float>& buffer)
    {
        const int numCh = buffer.getNumChannels();
        if (numCh <= 1)
        {
            if (numCh < 1)
                buffer.setSize (1, juce::jmax (1, buffer.getNumSamples()));
            return;
        }

        juce::AudioBuffer<float> mono (1, buffer.getNumSamples());
        mono.copyFrom (0, 0, buffer, 0, 0, buffer.getNumSamples());
        for (int c = 1; c < numCh; ++c)
            mono.addFrom (0, 0, buffer, c, 0, buffer.getNumSamples(), 0.5f);
        mono.applyGain (0.5f + 0.5f / (float) numCh);
        buffer = std::move (mono);
    }

    void Soundboard::buildClip (Clip& clip, double rate)
    {
        clip.buffer.setSize (1, 1);
        if (clip.generator)
            clip.generator (clip.buffer, rate);
    }

    void Soundboard::buildUserClip (Clip& clip, double rate)
    {
        if (clip.source.getNumSamples() <= 0)
            return;

        clip.buffer.makeCopyOf (clip.source);
        if (clip.sourceRate > 0.0)
            resampleTo (clip.buffer, clip.sourceRate, rate);
        downmixToMono (clip.buffer);
    }

    void Soundboard::rebuildClips (double rate)
    {
        juce::SpinLock::ScopedLockType sl (lock);
        for (auto& c : clips)
        {
            if (c->generator)
                buildClip (*c, rate);
            else if (c->source.getNumSamples() > 0)
                buildUserClip (*c, rate);
        }
        sr = rate;
    }

    void Soundboard::addClip (juce::String name, std::function<void (juce::AudioBuffer<float>&, double)> generator)
    {
        auto clip = std::make_unique<Clip>();
        clip->name = std::move (name);
        clip->generator = generator;
        buildClip (*clip, prepared ? sr : 48000.0);

        juce::SpinLock::ScopedLockType sl (lock);
        clips.push_back (std::move (clip));
    }

    void Soundboard::buildDefaultKit()
    {
        {
            juce::SpinLock::ScopedLockType sl (lock);
            clips.clear();
        }

        auto makeEnv = [] (double t, double dur, double decay)
        {
            const double e = std::exp (-decay * t);
            const double fade = juce::jmin (1.0, t / 0.004) * juce::jmin (1.0, juce::jmax (0.0, (dur - t) / 0.02));
            return (float) (e * fade);
        };

        auto tone = [makeEnv] (juce::AudioBuffer<float>& buf, double rate, double dur, double f0, double f1,
                               double decay, float noiseAmt)
        {
            const int n = (int) (dur * rate);
            buf.setSize (1, juce::jmax (1, n));
            auto* d = buf.getWritePointer (0);
            double phase = 0.0;
            juce::Random rng;
            for (int i = 0; i < n; ++i)
            {
                const double t = i / rate;
                const double f = f0 + (f1 - f0) * (t / dur);
                phase += juce::MathConstants<double>::twoPi * f / rate;
                float s = (float) std::sin (phase);
                if (noiseAmt > 0.0f)
                    s = s * (1.0f - noiseAmt) + (rng.nextFloat() * 2.0f - 1.0f) * noiseAmt;
                d[i] = s * makeEnv (t, dur, decay);
            }
        };

        addClip ("LASER",  [tone] (juce::AudioBuffer<float>& b, double r) { tone (b, r, 0.35, 1500.0, 220.0, 6.0, 0.0f); });
        addClip ("ALERT",  [] (juce::AudioBuffer<float>& b, double rate)
        {
            const int n = (int) (0.36 * rate);
            b.setSize (1, n);
            auto* d = b.getWritePointer (0);
            for (int i = 0; i < n; ++i)
            {
                const double t = i / rate;
                const double f = (t < 0.18) ? 880.0 : 660.0;
                const float env = (float) (std::exp (-3.0 * std::fmod (t, 0.18)) * 0.8);
                d[i] = (float) std::sin (juce::MathConstants<double>::twoPi * f * t) * env;
            }
        });
        addClip ("BLIP",   [] (juce::AudioBuffer<float>& b, double rate)
        {
            const int n = (int) (0.08 * rate);
            b.setSize (1, n);
            auto* d = b.getWritePointer (0);
            for (int i = 0; i < n; ++i)
            {
                const double t = i / rate;
                const float s = (std::sin (juce::MathConstants<double>::twoPi * 1046.0 * t) >= 0.0) ? 0.6f : -0.6f;
                d[i] = s * (float) std::exp (-30.0 * t);
            }
        });
        addClip ("RISE",   [tone] (juce::AudioBuffer<float>& b, double r) { tone (b, r, 0.45, 180.0, 1600.0, 3.0, 0.02f); });
        addClip ("DROP",   [tone] (juce::AudioBuffer<float>& b, double r) { tone (b, r, 0.55, 900.0, 90.0, 3.0, 0.05f); });
        addClip ("DRUM",   [] (juce::AudioBuffer<float>& b, double rate)
        {
            const int n = (int) (0.28 * rate);
            b.setSize (1, n);
            auto* d = b.getWritePointer (0);
            juce::Random rng;
            for (int i = 0; i < n; ++i)
            {
                const double t = i / rate;
                const float noise = (rng.nextFloat() * 2.0f - 1.0f) * (float) std::exp (-22.0 * t);
                const float thump = (float) std::sin (juce::MathConstants<double>::twoPi * (110.0 - 70.0 * t) * t) * (float) std::exp (-9.0 * t);
                d[i] = juce::jlimit (-1.0f, 1.0f, noise * 0.5f + thump * 0.9f);
            }
        });
        addClip ("CHIME",  [] (juce::AudioBuffer<float>& b, double rate)
        {
            const int n = (int) (0.9 * rate);
            b.setSize (1, n);
            auto* d = b.getWritePointer (0);
            const double freqs[] = { 523.25, 783.99, 1046.5 };
            for (int i = 0; i < n; ++i)
            {
                const double t = i / rate;
                float s = 0.0f;
                for (int k = 0; k < 3; ++k)
                    s += (float) std::sin (juce::MathConstants<double>::twoPi * freqs[k] * t) * (float) std::exp (-(3.0 + k) * t) / 3.0f;
                d[i] = s;
            }
        });
        addClip ("STATIC", [] (juce::AudioBuffer<float>& b, double rate)
        {
            const int n = (int) (0.30 * rate);
            b.setSize (1, n);
            auto* d = b.getWritePointer (0);
            juce::Random rng;
            for (int i = 0; i < n; ++i)
            {
                const double t = i / rate;
                d[i] = (rng.nextFloat() * 2.0f - 1.0f) * 0.5f * (float) std::exp (-6.0 * t);
            }
        });
    }

    int Soundboard::addUserSound (const juce::File& file)
    {
        {
            juce::SpinLock::ScopedLockType sl (lock);
            for (int i = 0; i < (int) clips.size(); ++i)
                if (clips[(size_t) i]->path == file.getFullPathName())
                    return i; // already loaded
        }

        auto reader = std::unique_ptr<juce::AudioFormatReader> (formatManager.createReaderFor (file));
        if (reader == nullptr)
            return -1;

        auto clip = std::make_unique<Clip>();
        clip->name = file.getFileNameWithoutExtension().toUpperCase();
        clip->path = file.getFullPathName();
        clip->userAdded = true;
        clip->sourceRate = reader->sampleRate;
        clip->source.setSize (juce::jmax (1, (int) reader->numChannels), (int) reader->lengthInSamples);
        reader->read (&clip->source, 0, (int) reader->lengthInSamples, 0, true, true);

        // Keep the decoded source at its native rate and derive the mono,
        // device-rate playback buffer from it.
        buildUserClip (*clip, prepared ? sr : 48000.0);

        juce::SpinLock::ScopedLockType sl (lock);
        clips.push_back (std::move (clip));
        return (int) clips.size() - 1;
    }

    void Soundboard::removeSlot (int index)
    {
        juce::SpinLock::ScopedLockType sl (lock);
        if (juce::isPositiveAndBelow (index, (int) clips.size()))
            clips.erase (clips.begin() + index);
    }

    bool Soundboard::isUserSlot (int index) const
    {
        return juce::isPositiveAndBelow (index, (int) clips.size()) && clips[(size_t) index]->userAdded;
    }

    bool Soundboard::isFavorite (int index) const
    {
        return juce::isPositiveAndBelow (index, (int) clips.size()) && clips[(size_t) index]->favorite;
    }

    void Soundboard::setFavorite (int index, bool shouldBeFavorite)
    {
        if (juce::isPositiveAndBelow (index, (int) clips.size()))
            clips[(size_t) index]->favorite = shouldBeFavorite;
    }

    void Soundboard::toggleFavorite (int index)
    {
        if (juce::isPositiveAndBelow (index, (int) clips.size()))
            clips[(size_t) index]->favorite = ! clips[(size_t) index]->favorite;
    }

    void Soundboard::applyFavorites (const juce::StringArray& names)
    {
        for (auto& c : clips)
            c->favorite = names.contains (c->name);
    }

    void Soundboard::collectLibrary (juce::StringArray& userPaths, juce::StringArray& userNames,
                                     juce::StringArray& favorites) const
    {
        userPaths.clear();
        userNames.clear();
        favorites.clear();

        for (const auto& c : clips)
        {
            if (c->userAdded && c->path.isNotEmpty())
            {
                userPaths.add (c->path);
                userNames.add (c->name);
            }
            if (c->favorite)
                favorites.add (c->name);
        }
    }

    void Soundboard::trigger (int slot, float velocity)
    {
        if (! juce::isPositiveAndBelow (slot, (int) clips.size()))
            return;
        auto& c = *clips[(size_t) slot];
        c.velocity.store (velocity);
        c.pos.store (0);
        c.playing.store (true);
    }

    void Soundboard::stopAll()
    {
        for (auto& c : clips)
        {
            c->playing.store (false);
            c->pos.store (0);
        }
    }

    juce::String Soundboard::getSlotName (int slot) const
    {
        if (juce::isPositiveAndBelow (slot, (int) clips.size()))
            return clips[(size_t) slot]->name;
        return {};
    }

    bool Soundboard::isPlaying (int slot) const
    {
        if (juce::isPositiveAndBelow (slot, (int) clips.size()))
            return clips[(size_t) slot]->playing.load();
        return false;
    }

    float Soundboard::getProgress (int slot) const
    {
        if (! juce::isPositiveAndBelow (slot, (int) clips.size()))
            return 0.0f;
        const auto& c = *clips[(size_t) slot];
        const int len = c.buffer.getNumSamples();
        if (len <= 0)
            return 0.0f;
        return juce::jlimit (0.0f, 1.0f, (float) c.pos.load() / (float) len);
    }

    void Soundboard::renderInto (juce::AudioBuffer<float>& out, float gainDb)
    {
        juce::SpinLock::ScopedTryLockType sl (lock);
        if (! sl.isLocked())
            return; // a clip is being added/removed; skip this block

        const float gain = juce::Decibels::decibelsToGain (gainDb);
        const int n = out.getNumSamples();
        const int outCh = out.getNumChannels();

        for (auto& c : clips)
        {
            if (! c->playing.load())
                continue;

            const int len = c->buffer.getNumSamples();
            if (len <= 0)
                continue;

            const float vel = c->velocity.load();
            int pos = c->pos.load();
            if (pos < 0 || pos >= len)
                pos = 0;

            const int toCopy = juce::jmin (n, len - pos);
            const auto* src = c->buffer.getReadPointer (0);

            for (int ch = 0; ch < outCh; ++ch)
            {
                auto* d = out.getWritePointer (ch);
                for (int i = 0; i < toCopy; ++i)
                    d[i] += src[pos + i] * vel * gain;
            }

            pos += toCopy;
            if (pos >= len)
            {
                c->playing.store (false);
                c->pos.store (0);
            }
            else
            {
                c->pos.store (pos);
            }
        }
    }
}
