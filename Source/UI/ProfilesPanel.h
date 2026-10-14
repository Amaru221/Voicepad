#pragma once

#include "Panel.h"
#include "Config/ConfigManager.h"

namespace vp
{
    // Save / load / delete named configuration profiles.
    class ProfilesPanel : public Panel
    {
    public:
        explicit ProfilesPanel (ConfigManager& configToUse);

        void resized() override;

        // Capture the current live state into a Profile (the panel sets the name).
        std::function<Profile()> onCapture;
        // Apply a loaded Profile to the live state.
        std::function<void (const Profile&)> onApply;
        std::function<void (const juce::String&)> onLog;

        void refresh();

    private:
        void doSave();
        void doLoad();
        void doDelete();

        ConfigManager& config;
        juce::Label nameLabel, listLabel;
        juce::TextEditor nameEditor;
        juce::ComboBox profileBox;
        juce::TextButton saveButton { "SAVE" };
        juce::TextButton loadButton { "LOAD" };
        juce::TextButton deleteButton { "DELETE" };
    };
}
