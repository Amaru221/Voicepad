#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include "Common.h"

namespace vp
{
    // Pad sampler: built-in synthesized kit + user-loaded sounds, favourites,
    // rendering into the voice stream. UI mutations are locked against the audio thread.
    class Soundboard
    {
    public:
        Soundboard() = default;

        void prepare (double sampleRate, int blockSize);
        void buildDefaultKit();

        int  addUserSound (const juce::File& file);   // returns slot index, or -1
        void removeSlot (int index);
        bool isUserSlot (int index) const;
        bool isFavorite (int index) const;
        void setFavorite (int index, bool shouldBeFavorite);
        void toggleFavorite (int index);
        void applyFavorites (const juce::StringArray& names);

        void collectLibrary (juce::StringArray& userPaths, juce::StringArray& userNames,
                             juce::StringArray& favorites) const;

        void trigger (int slot, float velocity = 1.0f);
        void stopAll();

        // Called on the audio thread.
        void renderInto (juce::AudioBuffer<float>& out, float gainDb);

        int getNumSlots() const noexcept { return (int) clips.size(); }
        juce::String getSlotName (int slot) const;
        bool isPlaying (int slot) const;
        float getProgress (int slot) const;

    private:
        struct Clip
        {
            juce::String name;
            juce::String path;
            bool userAdded = false;
            bool favorite = false;
            juce::AudioBuffer<float> buffer;
            // Built-in clips keep their generator so they can be regenerated for
            // a different device sample rate.
            std::function<void (juce::AudioBuffer<float>&, double)> generator;
            // User clips keep the decoded source (at its native rate) so they can
            // be re-resampled if the device sample rate changes.
            juce::AudioBuffer<float> source;
            double sourceRate = 0.0;
            std::atomic<bool>  playing { false };
            std::atomic<int>   pos { 0 };
            std::atomic<float> velocity { 1.0f };
        };

        void addClip (juce::String name, std::function<void (juce::AudioBuffer<float>&, double)> generator);
        static void resampleTo (juce::AudioBuffer<float>& buffer, double fromRate, double toRate);
        static void downmixToMono (juce::AudioBuffer<float>& buffer);

        void buildClip (Clip& clip, double rate);
        void buildUserClip (Clip& clip, double rate);
        void rebuildClips (double rate);

        std::vector<std::unique_ptr<Clip>> clips;
        juce::AudioFormatManager formatManager;
        juce::SpinLock lock;
        double sr = 48000.0;
        int blockSize = 512;
        bool prepared = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Soundboard)
    };
}
