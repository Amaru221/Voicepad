#include "ConfigManager.h"

namespace vp
{
    void Profile::captureFrom (const VoiceParams& p)
    {
        inputGainDb      = p.inputGainDb.load();
        gateThreshold    = p.gateThreshold.load();
        highpassHz       = p.highpassHz.load();
        pitchSemis       = p.pitchSemis.load();
        formantSemis     = p.formantSemis.load();
        drive            = p.drive.load();
        robotMix         = p.robotMix.load();
        robotFreq        = p.robotFreq.load();
        chorusMix        = p.chorusMix.load();
        chorusRate       = p.chorusRate.load();
        chorusDepth      = p.chorusDepth.load();
        echoMix          = p.echoMix.load();
        echoTimeMs       = p.echoTimeMs.load();
        echoFeedback     = p.echoFeedback.load();
        reverbMix        = p.reverbMix.load();
        compAmount       = p.compAmount.load();
        outputGainDb     = p.outputGainDb.load();
        soundboardGainDb = p.soundboardGainDb.load();
        noiseCancel      = p.noiseCancel.load();
        noiseThresholdDb = p.noiseThresholdDb.load();
    }

    void Profile::applyTo (VoiceParams& p) const
    {
        p.inputGainDb.store (inputGainDb);
        p.gateThreshold.store (gateThreshold);
        p.highpassHz.store (highpassHz);
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
        p.soundboardGainDb.store (soundboardGainDb);
        p.noiseCancel.store (noiseCancel);
        p.noiseThresholdDb.store (noiseThresholdDb);
    }

    juce::var Profile::toVar() const
    {
        juce::DynamicObject::Ptr o = new juce::DynamicObject();
        o->setProperty ("name", name);
        o->setProperty ("selectedPreset", selectedPreset);
        o->setProperty ("inputDevice", inputDevice);
        o->setProperty ("outputDevice", outputDevice);
        o->setProperty ("inputGainDb", inputGainDb);
        o->setProperty ("gateThreshold", gateThreshold);
        o->setProperty ("highpassHz", highpassHz);
        o->setProperty ("pitchSemis", pitchSemis);
        o->setProperty ("formantSemis", formantSemis);
        o->setProperty ("drive", drive);
        o->setProperty ("robotMix", robotMix);
        o->setProperty ("robotFreq", robotFreq);
        o->setProperty ("chorusMix", chorusMix);
        o->setProperty ("chorusRate", chorusRate);
        o->setProperty ("chorusDepth", chorusDepth);
        o->setProperty ("echoMix", echoMix);
        o->setProperty ("echoTimeMs", echoTimeMs);
        o->setProperty ("echoFeedback", echoFeedback);
        o->setProperty ("reverbMix", reverbMix);
        o->setProperty ("compAmount", compAmount);
        o->setProperty ("outputGainDb", outputGainDb);
        o->setProperty ("soundboardGainDb", soundboardGainDb);
        o->setProperty ("noiseCancel", noiseCancel);
        o->setProperty ("noiseThresholdDb", noiseThresholdDb);
        return juce::var (o.get());
    }

    Profile Profile::fromVar (const juce::var& v)
    {
        Profile p;
        auto* o = v.getDynamicObject();
        if (o == nullptr)
            return p;

        auto f = [o] (const char* key, float def)
        {
            return o->hasProperty (key) ? (float) (double) o->getProperty (key) : def;
        };

        p.name            = o->getProperty ("name").toString();
        p.selectedPreset  = (int) o->getProperty ("selectedPreset");
        p.inputDevice     = o->getProperty ("inputDevice").toString();
        p.outputDevice    = o->getProperty ("outputDevice").toString();
        p.inputGainDb     = f ("inputGainDb", 0.0f);
        p.gateThreshold   = f ("gateThreshold", -55.0f);
        p.highpassHz      = f ("highpassHz", 90.0f);
        p.pitchSemis      = f ("pitchSemis", 0.0f);
        p.formantSemis    = f ("formantSemis", 0.0f);
        p.drive           = f ("drive", 0.0f);
        p.robotMix        = f ("robotMix", 0.0f);
        p.robotFreq       = f ("robotFreq", 55.0f);
        p.chorusMix       = f ("chorusMix", 0.0f);
        p.chorusRate      = f ("chorusRate", 1.2f);
        p.chorusDepth     = f ("chorusDepth", 0.35f);
        p.echoMix         = f ("echoMix", 0.0f);
        p.echoTimeMs      = f ("echoTimeMs", 240.0f);
        p.echoFeedback    = f ("echoFeedback", 0.32f);
        p.reverbMix       = f ("reverbMix", 0.0f);
        p.compAmount      = f ("compAmount", 0.25f);
        p.outputGainDb    = f ("outputGainDb", 0.0f);
        p.soundboardGainDb = f ("soundboardGainDb", 0.0f);
        p.noiseCancel     = (bool) o->getProperty ("noiseCancel");
        p.noiseThresholdDb = f ("noiseThresholdDb", -45.0f);
        return p;
    }

    juce::File ConfigManager::configDir() const
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                    .getChildFile ("VoicePad");
    }

    juce::File ConfigManager::appStateFile() const { return configDir().getChildFile ("config.json"); }
    juce::File ConfigManager::profilesFile() const { return configDir().getChildFile ("profiles.json"); }
    juce::File ConfigManager::themeFile() const { return configDir().getChildFile ("theme.json"); }
    juce::File ConfigManager::soundsFile() const { return configDir().getChildFile ("sounds.json"); }

    void ConfigManager::saveAppState (int selectedPreset, const juce::String& inDev,
                                      const juce::String& outDev, const VoiceParams& p)
    {
        Profile snapshot;
        snapshot.captureFrom (p);
        snapshot.selectedPreset = selectedPreset;
        snapshot.inputDevice = inDev;
        snapshot.outputDevice = outDev;

        auto file = appStateFile();
        file.getParentDirectory().createDirectory();
        file.replaceWithText (juce::JSON::toString (snapshot.toVar(), true));
    }

    void ConfigManager::loadAppState (int& selectedPreset, juce::String& inDev,
                                      juce::String& outDev, VoiceParams& p)
    {
        auto file = appStateFile();
        if (! file.existsAsFile())
            return;

        juce::var parsed;
        if (juce::JSON::parse (file.loadFileAsString(), parsed).wasOk())
        {
            auto prof = Profile::fromVar (parsed);
            selectedPreset = prof.selectedPreset;
            inDev = prof.inputDevice;
            outDev = prof.outputDevice;
            prof.applyTo (p);
        }
    }

    juce::String ConfigManager::getProfileName (int index) const
    {
        if (juce::isPositiveAndBelow (index, (int) profiles.size()))
            return profiles[(size_t) index].name;
        return {};
    }

    Profile ConfigManager::getProfile (int index) const
    {
        if (juce::isPositiveAndBelow (index, (int) profiles.size()))
            return profiles[(size_t) index];
        return {};
    }

    juce::StringArray ConfigManager::getProfileNames() const
    {
        juce::StringArray names;
        for (const auto& p : profiles)
            names.add (p.name);
        return names;
    }

    void ConfigManager::addOrUpdateProfile (const Profile& p)
    {
        for (auto& existing : profiles)
        {
            if (existing.name.equalsIgnoreCase (p.name))
            {
                existing = p;
                return;
            }
        }
        profiles.push_back (p);
    }

    void ConfigManager::removeProfile (int index)
    {
        if (juce::isPositiveAndBelow (index, (int) profiles.size()))
            profiles.erase (profiles.begin() + index);
    }

    void ConfigManager::loadProfiles()
    {
        profiles.clear();
        auto file = profilesFile();
        if (! file.existsAsFile())
            return;

        juce::var parsed;
        if (! juce::JSON::parse (file.loadFileAsString(), parsed).wasOk())
            return;

        if (auto* arr = parsed.getArray())
            for (const auto& v : *arr)
                profiles.push_back (Profile::fromVar (v));
    }

    void ConfigManager::saveProfiles() const
    {
        juce::Array<juce::var> arr;
        for (const auto& p : profiles)
            arr.add (p.toVar());

        auto file = profilesFile();
        file.getParentDirectory().createDirectory();
        file.replaceWithText (juce::JSON::toString (juce::var (arr), true));
    }

    void ConfigManager::saveTheme (int index)
    {
        juce::DynamicObject::Ptr o = new juce::DynamicObject();
        o->setProperty ("themeIndex", index);

        auto file = themeFile();
        file.getParentDirectory().createDirectory();
        file.replaceWithText (juce::JSON::toString (juce::var (o.get()), true));
    }

    int ConfigManager::loadTheme() const
    {
        auto file = themeFile();
        if (! file.existsAsFile())
            return 0;

        juce::var parsed;
        if (juce::JSON::parse (file.loadFileAsString(), parsed).wasOk())
            if (auto* o = parsed.getDynamicObject())
                return (int) o->getProperty ("themeIndex");

        return 0;
    }

    void ConfigManager::saveSoundLibrary (const juce::StringArray& userPaths, const juce::StringArray& userNames,
                                          const juce::StringArray& favorites)
    {
        juce::Array<juce::var> users;
        for (int i = 0; i < juce::jmin (userPaths.size(), userNames.size()); ++i)
        {
            juce::DynamicObject::Ptr o = new juce::DynamicObject();
            o->setProperty ("path", userPaths[i]);
            o->setProperty ("name", userNames[i]);
            users.add (juce::var (o.get()));
        }

        juce::Array<juce::var> favs;
        for (const auto& f : favorites)
            favs.add (f);

        juce::DynamicObject::Ptr root = new juce::DynamicObject();
        root->setProperty ("users", juce::var (users));
        root->setProperty ("favorites", juce::var (favs));

        auto file = soundsFile();
        file.getParentDirectory().createDirectory();
        file.replaceWithText (juce::JSON::toString (juce::var (root.get()), true));
    }

    void ConfigManager::loadSoundLibrary (juce::StringArray& userPaths, juce::StringArray& userNames,
                                          juce::StringArray& favorites) const
    {
        userPaths.clear();
        userNames.clear();
        favorites.clear();

        auto file = soundsFile();
        if (! file.existsAsFile())
            return;

        juce::var parsed;
        if (! juce::JSON::parse (file.loadFileAsString(), parsed).wasOk())
            return;

        if (auto* o = parsed.getDynamicObject())
        {
            if (auto* users = o->getProperty ("users").getArray())
                for (const auto& v : *users)
                    if (auto* u = v.getDynamicObject())
                    {
                        userPaths.add (u->getProperty ("path").toString());
                        userNames.add (u->getProperty ("name").toString());
                    }

            if (auto* favs = o->getProperty ("favorites").getArray())
                for (const auto& v : *favs)
                    favorites.add (v.toString());
        }
    }
}
