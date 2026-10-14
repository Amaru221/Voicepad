#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <memory>
#include <vector>
#include "Common.h"
#include "VoiceParams.h"
#include "DspChain.h"
#include "Soundboard/Soundboard.h"

namespace vp
{
    // Lock-free-ish ring buffer for the scope visualiser.
    struct ScopeData
    {
        static constexpr int size = 4096;
        std::array<float, size> samples {};
        std::atomic<int> writePos { 0 };
        mutable juce::SpinLock lock;

        void pushBlock (const float* data, int n)
        {
            juce::SpinLock::ScopedTryLockType sl (lock);
            if (! sl.isLocked())
                return;

            int wp = writePos.load();
            for (int i = 0; i < n; ++i)
            {
                samples[(size_t) wp] = data[i];
                wp = (wp + 1) % size;
            }
            writePos.store (wp);
        }

        void snapshot (std::vector<float>& dest) const
        {
            juce::SpinLock::ScopedLockType sl (lock);
            dest.resize ((size_t) size);
            const int wp = writePos.load();
            for (int i = 0; i < size; ++i)
                dest[(size_t) i] = samples[(size_t) ((wp + i) % size)];
        }
    };

    class AudioEngine : public juce::AudioIODeviceCallback
    {
    public:
        AudioEngine (VoiceParams& params, DspChain& chain, Soundboard& boardToUse);
        ~AudioEngine() override;

        void initialise();

        juce::AudioDeviceManager& getDeviceManager() noexcept { return deviceManager; }

        juce::StringArray getInputDeviceNames() const;
        juce::StringArray getOutputDeviceNames() const;

        void applySetup (const juce::String& inputName, const juce::String& outputName);
        bool autoRoute();
        void startAudio();
        void stopAudio();
        bool isRunning() const noexcept { return running.load(); }

        float getInputLevel() const noexcept  { return inputLevel.load(); }
        float getOutputLevel() const noexcept { return outputLevel.load(); }
        bool  getVoiceActive() const noexcept { return voiceActive.load(); }
        ScopeData& getScope() noexcept         { return scope; }

        double getCurrentSampleRate() const noexcept { return currentSampleRate.load(); }
        int    getCurrentBlockSize() const noexcept  { return currentBlockSize.load(); }

        juce::String getInputDeviceName() const;
        juce::String getOutputDeviceName() const;

        std::function<void (const juce::String&)> onLog;

        void audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                               float* const* outputChannelData, int numOutputChannels,
                                               int numSamples, const juce::AudioIODeviceCallbackContext&) override;
        void audioDeviceAboutToStart (juce::AudioIODevice*) override;
        void audioDeviceStopped() override;

    private:
        void log (const juce::String& msg) { if (onLog != nullptr) onLog (msg); }

        VoiceParams& params;
        DspChain& dspChain;
        Soundboard& soundboard;
        juce::AudioDeviceManager deviceManager;

        std::atomic<bool> running { false };
        std::atomic<float> inputLevel { 0.0f };
        std::atomic<float> outputLevel { 0.0f };
        std::atomic<bool> voiceActive { false };
        std::atomic<double> currentSampleRate { 48000.0 };
        std::atomic<int> currentBlockSize { 512 };

        juce::AudioBuffer<float> workBuffer;
        std::vector<float> monoScratch;
        ScopeData scope;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioEngine)
    };
}
