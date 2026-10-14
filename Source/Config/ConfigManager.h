#pragma once

#include "Common.h"
#include "Audio/VoiceParams.h"

namespace vp
{
    // A full named configuration: routing + every voice parameter.
    struct Profile
    {
        juce::String name;
        int selectedPreset = 0;
        juce::String inputDevice, outputDevice;

        float inputGainDb    = 0.0f;
        float gateThreshold  = -55.0f;
        float highpassHz     = 90.0f;
        float pitchSemis     = 0.0f;
        float formantSemis   = 0.0f;
        float drive          = 0.0f;
        float robotMix       = 0.0f;
        float robotFreq      = 55.0f;
        float chorusMix      = 0.0f;
        float chorusRate     = 1.2f;
        float chorusDepth    = 0.35f;
        float echoMix        = 0.0f;
        float echoTimeMs     = 240.0f;
        float echoFeedback   = 0.32f;
        float reverbMix      = 0.0f;
        float compAmount     = 0.25f;
        float outputGainDb   = 0.0f;
        float soundboardGainDb = -3.0f;
        bool  noiseCancel    = false;
        float noiseThresholdDb = -45.0f;

        void captureFrom (const VoiceParams& p);
        void applyTo (VoiceParams& p) const;

        juce::var toVar() const;
        static Profile fromVar (const juce::var& v);
    };

    // Persists the app state (routing + live parameters) and the named profiles.
    class ConfigManager
    {
    public:
        ConfigManager() = default;

        void saveAppState (int selectedPreset, const juce::String& inDev,
                           const juce::String& outDev, const VoiceParams& p);
        void loadAppState (int& selectedPreset, juce::String& inDev,
                           juce::String& outDev, VoiceParams& p);

        int getNumProfiles() const noexcept { return (int) profiles.size(); }
        juce::String getProfileName (int index) const;
        Profile getProfile (int index) const;
        juce::StringArray getProfileNames() const;

        void addOrUpdateProfile (const Profile& p);
        void removeProfile (int index);

        void loadProfiles();
        void saveProfiles() const;

        void saveTheme (int index);
        int  loadTheme() const;

        void saveSoundLibrary (const juce::StringArray& userPaths, const juce::StringArray& userNames,
                               const juce::StringArray& favorites);
        void loadSoundLibrary (juce::StringArray& userPaths, juce::StringArray& userNames,
                               juce::StringArray& favorites) const;

    private:
        juce::File configDir() const;
        juce::File appStateFile() const;
        juce::File profilesFile() const;
        juce::File themeFile() const;
        juce::File soundsFile() const;

        std::vector<Profile> profiles;
    };
}
