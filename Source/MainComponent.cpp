#include "MainComponent.h"

namespace vp
{
    namespace
    {
        // CRT scanline overlay drawn above the whole mosaic (mouse-transparent).
        class CrtOverlay : public juce::Component
        {
        public:
            CrtOverlay()
            {
                setInterceptsMouseClicks (false, false);
            }

            void paint (juce::Graphics& g) override
            {
                g.setColour (juce::Colours::black.withAlpha (0.16f));
                for (int y = 0; y < getHeight(); y += 3)
                    g.fillRect (0, y, getWidth(), 1);

                juce::ColourGradient glow (Palette::amber().withAlpha (0.05f),
                                           (float) getWidth() * 0.5f, 0.0f,
                                           juce::Colours::transparentBlack,
                                           (float) getWidth() * 0.5f, (float) getHeight(), false);
                g.setGradientFill (glow);
                g.fillAll();
            }
        };
    }

    MainComponent::MainComponent()
        : logPanel ("EVENT REGISTRY", logColumns()),
          healthPanel ("SOURCE HEALTH", healthColumns())
    {
        juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

        addAndMakeVisible (toolbar);
        addAndMakeVisible (devicePanel);
        addAndMakeVisible (presetPanel);
        addAndMakeVisible (effectsPanel);
        addAndMakeVisible (scopePanel);
        addAndMakeVisible (soundboardPanel);
        addAndMakeVisible (profilesPanel);
        addAndMakeVisible (logPanel);
        addAndMakeVisible (healthPanel);
        addAndMakeVisible (manualPanel);

        addAndMakeVisible (splitLeft);
        addAndMakeVisible (splitRight);
        addAndMakeVisible (splitBottom);
        addAndMakeVisible (splitProfiles);
        addAndMakeVisible (splitSoundboard);

        auto* overlay = new CrtOverlay();
        overlay->setComponentID ("crt");
        addAndMakeVisible (overlay);
        overlay->setBounds (getLocalBounds());

        // ---- Core callbacks -------------------------------------------------
        engine.onLog = [this] (const juce::String& s) { logMessage ("ENGINE", s, 0); };
        devicePanel.onLog = [this] (const juce::String& s) { logMessage ("ROUTER", s, 0); };

        toolbar.onPowerToggled = [this]
        {
            logMessage ("VOICE", params.bypass.load() ? "PROCESSING BYPASSED" : "PROCESSING ACTIVE", 1);
        };
        toolbar.onThemeChanged = [this] (int index) { changeTheme (index); };

        presetPanel.onPresetApplied = [this] (int index)
        {
            logMessage ("PRESET", presetManager.getDescriptor (index).name, 0);
        };

        soundboardPanel.onTriggered = [this] (int index)
        {
            logMessage ("BOARD", soundboard.getSlotName (index), 0);
        };

        soundboardPanel.onLibraryChanged = [this] { saveSoundLibrary(); };

        hotkeys.setActionCallback ([this] (int action) { handleHotkey (action); });

        // ---- Section RESET / SAVE buttons ----------------------------------
        devicePanel.setSectionActions (true, true);
        devicePanel.onResetSection = [this]
        {
            if (engine.autoRoute())
                devicePanel.refreshLists();
            logMessage ("ROUTER", "SECTION RESET", 0);
        };
        devicePanel.onSaveSection = [this] { saveAppState(); logMessage ("ROUTER", "SECTION SAVED", 0); };

        presetPanel.setSectionActions (true, true);
        presetPanel.onResetSection = [this] { presetPanel.selectPreset (0, true); };
        presetPanel.onSaveSection = [this] { saveAppState(); logMessage ("PRESET", "SECTION SAVED", 0); };

        effectsPanel.setSectionActions (true, true);
        effectsPanel.onResetSection = [this]
        {
            presetManager.applyPreset (presetManager.getSelected(), params);
            logMessage ("EFFECTS", "RESET TO PRESET VALUES", 0);
        };
        effectsPanel.onSaveSection = [this] { saveAppState(); logMessage ("EFFECTS", "SECTION SAVED", 0); };

        soundboardPanel.setSectionActions (true, true);
        soundboardPanel.onResetSection = [this] { soundboard.stopAll(); logMessage ("BOARD", "CLIPS STOPPED", 0); };
        soundboardPanel.onSaveSection = [this] { saveAppState(); saveSoundLibrary(); logMessage ("BOARD", "LIBRARY SAVED", 0); };

        scopePanel.setSectionActions (true, true);
        scopePanel.onResetSection = [this] { scopePanel.resetView(); logMessage ("SCOPE", "VIEW RESET", 0); };
        scopePanel.onSaveSection = [this] { saveAppState(); logMessage ("SCOPE", "SECTION SAVED", 0); };

        logPanel.setSectionActions (true, true);
        logPanel.onResetSection = [this] { logPanel.resetView(); };
        logPanel.onSaveSection = [this] { saveAppState(); };

        healthPanel.setSectionActions (false, true);
        healthPanel.onSaveSection = [this] { saveAppState(); };

        manualPanel.setSectionActions (false, false);

        // ---- Profiles -------------------------------------------------------
        profilesPanel.onCapture = [this] { return captureProfile(); };
        profilesPanel.onApply   = [this] (const Profile& p) { applyProfile (p); };
        profilesPanel.onLog     = [this] (const juce::String& s) { logMessage ("PROFILE", s, 1); };

        // ---- Splitters ------------------------------------------------------
        splitLeft.onDrag = [this] (int delta)
        {
            leftWidth = juce::jlimit (200, juce::jmax (200, getWidth() - 640), leftWidth + delta);
            resized();
        };
        splitRight.onDrag = [this] (int delta)
        {
            rightWidth = juce::jlimit (260, juce::jmax (260, getWidth() - 640), rightWidth - delta);
            resized();
        };
        splitBottom.onDrag = [this] (int delta)
        {
            healthHeight = juce::jlimit (90, juce::jmax (90, getHeight() / 2), healthHeight - delta);
            resized();
        };
        splitProfiles.onDrag = [this] (int delta)
        {
            profilesWidth = juce::jlimit (260, juce::jmax (260, getWidth() - 420), profilesWidth - delta);
            resized();
        };
        splitSoundboard.onDrag = [this] (int delta)
        {
            soundboardHeight = juce::jlimit (120, juce::jmax (120, getHeight() - 300), soundboardHeight + delta);
            resized();
        };

        // ---- Boot sequence --------------------------------------------------
        engine.initialise();

        ThemeManager::setCurrentIndex (config.loadTheme());
        lookAndFeel.applyTheme();
        toolbar.setThemeIndex (ThemeManager::getCurrentIndex());

        config.loadProfiles();
        profilesPanel.refresh();

        int sel = 0;
        juce::String inDev, outDev;
        config.loadAppState (sel, inDev, outDev, params);
        presetManager.setSelected (sel);
        if (inDev.isNotEmpty() || outDev.isNotEmpty())
            engine.applySetup (inDev, outDev);
        devicePanel.refreshLists();
        presetPanel.highlightOnly (sel);

        soundboard.prepare (48000.0, 512);
        soundboard.buildDefaultKit();

        {
            juce::StringArray userPaths, userNames, favs;
            config.loadSoundLibrary (userPaths, userNames, favs);
            for (const auto& p : userPaths)
                soundboard.addUserSound (juce::File (p));
            soundboard.applyFavorites (favs);
            soundboardPanel.refreshAll();
        }

        logMessage ("SYSTEM", "VOICEPAD CORE ONLINE", 0);
        logMessage ("SYSTEM", "SELECT CABLE INPUT AS ROUTE OUTPUT", 2);

        setSize (1480, 920);
        startTimerHz (4);
    }

    MainComponent::~MainComponent()
    {
        stopTimer();
        engine.stopAudio();
        saveAppState();
        juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    }

    std::vector<TablePanel::Column> MainComponent::logColumns()
    {
        return {
            { "TIME", 82 }, { "TYPE", 74 }, { "SOURCE", 80 }, { "MESSAGE", 300 }, { "LVL", 40 }
        };
    }

    std::vector<TablePanel::Column> MainComponent::healthColumns()
    {
        return {
            { "DEVICE", 210 }, { "ROLE", 74 }, { "STATE", 64 }, { "RATE", 62 }, { "BUF", 52 }, { "RMS", 70 }
        };
    }

    void MainComponent::logMessage (const juce::String& source, const juce::String& message, int level)
    {
        eventLog.add (level == 2 ? "ALERT" : (level == 1 ? "EVENT" : "INFO"), source, message, level);
        lastLogCount = -1;
    }

    void MainComponent::timerCallback()
    {
        updateLogRows();
        updateHealthRows();
    }

    void MainComponent::updateLogRows()
    {
        const auto& entries = eventLog.getEntries();
        if ((int) entries.size() == lastLogCount)
            return;
        lastLogCount = (int) entries.size();

        juce::Array<juce::StringArray> rows;
        const int start = juce::jmax (0, lastLogCount - 300);
        for (int i = start; i < lastLogCount; ++i)
        {
            const auto& e = entries[(size_t) i];
            const auto time = juce::Time ((juce::int64) (e.time * 1000.0));
            rows.add ({ time.toString (false, true, true, false), e.type, e.source, e.message,
                        juce::String (e.level) });
        }
        logPanel.setRows (rows);
    }

    void MainComponent::updateHealthRows()
    {
        juce::Array<juce::StringArray> rows;
        const bool running = engine.isRunning();
        const juce::String rate = juce::String (engine.getCurrentSampleRate(), 0);
        const juce::String buf = juce::String (engine.getCurrentBlockSize());
        const float inRms = engine.getInputLevel();
        const float outRms = engine.getOutputLevel();

        auto rmsText = [] (float v) { return juce::String (juce::Decibels::gainToDecibels (juce::jmax (1.0e-5f, v)), 1) + "dB"; };

        rows.add ({ engine.getInputDeviceName(), "CAPTURE", (running ? "LIVE" : "IDLE"), rate, buf, rmsText (inRms) });
        rows.add ({ engine.getOutputDeviceName(), "RENDER", (running ? "LIVE" : "IDLE"), rate, buf, rmsText (outRms) });
        rows.add ({ "VOICEPAD CORE", "PROCESS", (params.bypass.load() ? "BYPASS" : "ACTIVE"), "-", "-",
                    presetManager.getDescriptor (presetManager.getSelected()).name });
        rows.add ({ "THEME", "SKIN", ThemeManager::current().name, "-", "-",
                    juce::String (params.pitchSemis.load(), 1) + " st" });

        healthPanel.setRows (rows);
    }

    void MainComponent::handleHotkey (int action)
    {
        if (juce::isPositiveAndBelow (action, presetManager.getNumPresets()))
        {
            presetPanel.selectPreset (action, true);
        }
        else if (action == GlobalHotkeys::BypassAction)
        {
            params.bypass.store (! params.bypass.load());
            logMessage ("VOICE", params.bypass.load() ? "BYPASS ON" : "BYPASS OFF", 1);
        }
        else if (action == GlobalHotkeys::StopAllAction)
        {
            soundboard.stopAll();
            logMessage ("BOARD", "ALL CLIPS STOPPED", 1);
        }
    }

    void MainComponent::saveAppState()
    {
        config.saveAppState (presetManager.getSelected(),
                             engine.getInputDeviceName(),
                             engine.getOutputDeviceName(),
                             params);
    }

    void MainComponent::saveSoundLibrary()
    {
        juce::StringArray userPaths, userNames, favs;
        soundboard.collectLibrary (userPaths, userNames, favs);
        config.saveSoundLibrary (userPaths, userNames, favs);
    }

    Profile MainComponent::captureProfile()
    {
        Profile p;
        p.captureFrom (params);
        p.selectedPreset = presetManager.getSelected();
        p.inputDevice = engine.getInputDeviceName();
        p.outputDevice = engine.getOutputDeviceName();
        return p;
    }

    void MainComponent::applyProfile (const Profile& profile)
    {
        if (juce::isPositiveAndBelow (profile.selectedPreset, presetManager.getNumPresets()))
            presetPanel.selectPreset (profile.selectedPreset, false);

        profile.applyTo (params);

        if (profile.inputDevice.isNotEmpty() && profile.outputDevice.isNotEmpty())
            engine.applySetup (profile.inputDevice, profile.outputDevice);

        devicePanel.refreshLists();
    }

    void MainComponent::changeTheme (int index)
    {
        ThemeManager::setCurrentIndex (index);
        lookAndFeel.applyTheme();
        sendLookAndFeelChange();
        config.saveTheme (index);
        repaintAll (*this);
        logMessage ("THEME", "SKIN: " + ThemeManager::current().name, 0);
    }

    void MainComponent::repaintAll (juce::Component& c)
    {
        c.repaint();
        for (int i = 0; i < c.getNumChildComponents(); ++i)
            repaintAll (*c.getChildComponent (i));
    }

    void MainComponent::paint (juce::Graphics& g)
    {
        g.fillAll (Palette::bg());

        auto status = getLocalBounds().removeFromBottom (38).withTrimmedTop (16).reduced (8, 0);
        g.setColour (Palette::bgPanel());
        g.fillRect (status);
        g.setColour (Palette::amberFaint());
        g.drawRect (status, 1);

        g.setColour (Palette::amberDim());
        g.setFont (monoFont (9.5f));
        g.drawFittedText ("SYSTEM READY  //  SET DISCORD OR GAME MIC TO  \"CABLE OUTPUT\"  TO RECEIVE PROCESSED VOICE",
                          status.reduced (10, 0), juce::Justification::centredLeft, 1);

        g.setColour (engine.isRunning() ? Palette::ok() : Palette::alert());
        g.drawFittedText (engine.isRunning() ? "LINK ESTABLISHED" : "LINK DOWN",
                          status.reduced (10, 0), juce::Justification::centredRight, 1);
    }

    void MainComponent::resized()
    {
        auto area = getLocalBounds();

        toolbar.setBounds (area.removeFromTop (36));
        area.removeFromBottom (38); // status strip (painted in paint())

        auto bottomStrip = area.removeFromBottom (healthHeight);
        splitBottom.setBounds (area.removeFromBottom (5));

        auto leftCol = area.removeFromLeft (leftWidth);
        splitLeft.setBounds (area.removeFromLeft (5));

        auto rightCol = area.removeFromRight (rightWidth);
        splitRight.setBounds (area.removeFromRight (5));

        auto centerCol = area;

        // bottom strip: health | profiles
        auto profilesCol = bottomStrip.removeFromRight (profilesWidth);
        splitProfiles.setBounds (bottomStrip.removeFromRight (5));
        healthPanel.setBounds (bottomStrip.reduced (4, 2));
        profilesPanel.setBounds (profilesCol.reduced (4, 2));

        // left column
        devicePanel.setBounds (leftCol.removeFromTop (158).reduced (4, 2));
        presetPanel.setBounds (leftCol.reduced (4, 2));

        // center column
        scopePanel.setBounds (centerCol.removeFromTop (300).reduced (4, 2));
        effectsPanel.setBounds (centerCol.reduced (4, 2));

        // right column
        soundboardPanel.setBounds (rightCol.removeFromTop (soundboardHeight).reduced (4, 2));
        splitSoundboard.setBounds (rightCol.removeFromTop (5));
        manualPanel.setBounds (rightCol.removeFromBottom (182).reduced (4, 2));
        logPanel.setBounds (rightCol.reduced (4, 2));

        if (auto* ov = findChildWithID ("crt"))
        {
            ov->setBounds (getLocalBounds());
            ov->toFront (false);
        }
    }
}
