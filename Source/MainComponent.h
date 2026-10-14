#pragma once

#include "Common.h"
#include "EventLog.h"
#include "Audio/VoiceParams.h"
#include "Audio/DspChain.h"
#include "Audio/AudioEngine.h"
#include "Soundboard/Soundboard.h"
#include "Presets/PresetManager.h"
#include "Config/ConfigManager.h"
#include "Hotkeys/GlobalHotkeys.h"
#include "UI/ConsoleLookAndFeel.h"
#include "UI/Toolbar.h"
#include "UI/DevicePanel.h"
#include "UI/PresetPanel.h"
#include "UI/EffectsPanel.h"
#include "UI/ScopePanel.h"
#include "UI/SoundboardPanel.h"
#include "UI/ProfilesPanel.h"
#include "UI/TablePanel.h"
#include "UI/ManualPanel.h"
#include "UI/SplitterBar.h"

namespace vp
{
    class MainComponent : public juce::Component, private juce::Timer
    {
    public:
        MainComponent();
        ~MainComponent() override;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        void timerCallback() override;
        void logMessage (const juce::String& source, const juce::String& message, int level);
        void updateLogRows();
        void updateHealthRows();
        void handleHotkey (int action);

        void saveAppState();
        void saveSoundLibrary();
        Profile captureProfile();
        void applyProfile (const Profile& profile);
        void changeTheme (int index);
        void repaintAll (juce::Component& c);

        static std::vector<TablePanel::Column> logColumns();
        static std::vector<TablePanel::Column> healthColumns();

        VoiceParams params;
        DspChain dspChain { params };
        Soundboard soundboard;
        PresetManager presetManager;
        ConfigManager config;
        AudioEngine engine { params, dspChain, soundboard };
        GlobalHotkeys hotkeys;
        EventLog eventLog;
        ConsoleLookAndFeel lookAndFeel;

        Toolbar toolbar { engine, params };
        DevicePanel devicePanel { engine };
        PresetPanel presetPanel { presetManager, params, eventLog };
        EffectsPanel effectsPanel { params };
        ScopePanel scopePanel { engine };
        SoundboardPanel soundboardPanel { soundboard };
        ProfilesPanel profilesPanel { config };
        TablePanel logPanel;
        TablePanel healthPanel;
        ManualPanel manualPanel;

        SplitterBar splitLeft   { SplitterBar::Orientation::Vertical };
        SplitterBar splitRight  { SplitterBar::Orientation::Vertical };
        SplitterBar splitBottom { SplitterBar::Orientation::Horizontal };
        SplitterBar splitProfiles { SplitterBar::Orientation::Vertical };
        SplitterBar splitSoundboard { SplitterBar::Orientation::Horizontal };

        int leftWidth = 300;
        int rightWidth = 372;
        int healthHeight = 118;
        int profilesWidth = 400;
        int soundboardHeight = 200;

        int lastLogCount = -1;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
