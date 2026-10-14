#include "AudioEngine.h"

namespace vp
{
    AudioEngine::AudioEngine (VoiceParams& p, DspChain& chain, Soundboard& boardToUse)
        : params (p), dspChain (chain), soundboard (boardToUse)
    {
        workBuffer.setSize (2, 512, false, true, true);
        monoScratch.assign (512, 0.0f);
    }

    AudioEngine::~AudioEngine()
    {
        stopAudio();
    }

    void AudioEngine::initialise()
    {
        const auto err = deviceManager.initialiseWithDefaultDevices (1, 2);
        if (err.isNotEmpty())
            log ("DEVICE INIT: " + err);
        else
            log ("AUDIO SUBSYSTEM ONLINE");
    }

    juce::StringArray AudioEngine::getInputDeviceNames() const
    {
        if (auto* type = deviceManager.getCurrentDeviceTypeObject())
            return type->getDeviceNames (true);
        return {};
    }

    juce::StringArray AudioEngine::getOutputDeviceNames() const
    {
        if (auto* type = deviceManager.getCurrentDeviceTypeObject())
            return type->getDeviceNames (false);
        return {};
    }

    void AudioEngine::applySetup (const juce::String& inputName, const juce::String& outputName)
    {
        auto setup = deviceManager.getAudioDeviceSetup();

        if (inputName.isNotEmpty())
            setup.inputDeviceName = inputName;
        if (outputName.isNotEmpty())
            setup.outputDeviceName = outputName;

        setup.useDefaultInputChannels = true;
        setup.useDefaultOutputChannels = true;

        const auto err = deviceManager.setAudioDeviceSetup (setup, true);
        if (err.isNotEmpty())
            log ("ROUTE ERROR: " + err);
        else
            log ("ROUTE OK  IN[" + setup.inputDeviceName + "] OUT[" + setup.outputDeviceName + "]");
    }

    bool AudioEngine::autoRoute()
    {
        const auto outputs = getOutputDeviceNames();
        const auto inputs = getInputDeviceNames();

        juce::String cableOut;
        for (const auto& name : outputs)
            if (name.containsIgnoreCase ("CABLE Input") || name.containsIgnoreCase ("CABLE"))
            {
                cableOut = name;
                break;
            }

        if (cableOut.isEmpty())
        {
            log ("AUTOROUTE: VB-CABLE NOT FOUND");
            return false;
        }

        juce::String mic;
        for (const auto& name : inputs)
            if (! name.containsIgnoreCase ("CABLE"))
            {
                mic = name;
                break;
            }

        applySetup (mic, cableOut);
        return true;
    }

    void AudioEngine::startAudio()
    {
        if (running.load())
            return;
        deviceManager.addAudioCallback (this);
        running.store (true);
        log ("ENGINE STARTED");
    }

    void AudioEngine::stopAudio()
    {
        if (! running.load())
            return;
        deviceManager.removeAudioCallback (this);
        running.store (false);
        log ("ENGINE STOPPED");
    }

    juce::String AudioEngine::getInputDeviceName() const
    {
        return deviceManager.getAudioDeviceSetup().inputDeviceName;
    }

    juce::String AudioEngine::getOutputDeviceName() const
    {
        return deviceManager.getAudioDeviceSetup().outputDeviceName;
    }

    void AudioEngine::audioDeviceAboutToStart (juce::AudioIODevice* device)
    {
        const double sr = device->getCurrentSampleRate();
        const int block = device->getCurrentBufferSizeSamples();
        currentSampleRate.store (sr);
        currentBlockSize.store (block);

        workBuffer.setSize (2, juce::jmax (16, block), false, true, true);
        monoScratch.assign ((size_t) juce::jmax (16, block), 0.0f);

        dspChain.prepare (sr, block, 2);
        soundboard.prepare (sr, block);

        log ("STREAM " + juce::String (sr, 0) + " HZ / " + juce::String (block) + " SMP");
    }

    void AudioEngine::audioDeviceStopped()
    {
        dspChain.reset();
    }

    void AudioEngine::audioDeviceIOCallbackWithContext (const float* const* inputChannelData, int numInputChannels,
                                                        float* const* outputChannelData, int numOutputChannels,
                                                        int numSamples, const juce::AudioIODeviceCallbackContext&)
    {
        for (int c = 0; c < numOutputChannels; ++c)
            if (outputChannelData[c] != nullptr)
                juce::FloatVectorOperations::clear (outputChannelData[c], numSamples);

        if (workBuffer.getNumSamples() < numSamples)
            workBuffer.setSize (2, numSamples, false, false, true);

        workBuffer.clear (0, numSamples);

        // Input -> working stereo buffer (mono safe).
        for (int c = 0; c < 2; ++c)
        {
            const float* src = nullptr;
            if (c < numInputChannels) src = inputChannelData[c];
            else if (numInputChannels > 0) src = inputChannelData[0];

            if (src != nullptr)
                workBuffer.copyFrom (c, 0, src, numSamples);
        }

        const float rawLevel = workBuffer.getRMSLevel (0, 0, numSamples);
        inputLevel.store (rawLevel);
        voiceActive.store (rawLevel > 0.01f);

        dspChain.process (workBuffer);

        soundboard.renderInto (workBuffer, params.soundboardGainDb.load());

        outputLevel.store (workBuffer.getRMSLevel (0, 0, numSamples));

        // Scope feed (mono downmix).
        if (monoScratch.size() < (size_t) numSamples)
            monoScratch.resize ((size_t) numSamples);
        if (workBuffer.getNumChannels() >= 2)
        {
            const auto* l = workBuffer.getReadPointer (0);
            const auto* r = workBuffer.getReadPointer (1);
            for (int i = 0; i < numSamples; ++i)
                monoScratch[(size_t) i] = 0.5f * (l[i] + r[i]);
        }
        else
        {
            juce::FloatVectorOperations::copy (monoScratch.data(), workBuffer.getReadPointer (0), numSamples);
        }
        scope.pushBlock (monoScratch.data(), numSamples);

        // Working buffer -> outputs.
        for (int c = 0; c < numOutputChannels; ++c)
        {
            if (outputChannelData[c] == nullptr)
                continue;
            const float* src = workBuffer.getReadPointer (c < 2 ? c : 0);
            juce::FloatVectorOperations::copy (outputChannelData[c], src, numSamples);
        }
    }
}
