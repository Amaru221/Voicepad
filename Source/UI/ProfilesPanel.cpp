#include "ProfilesPanel.h"

namespace vp
{
    ProfilesPanel::ProfilesPanel (ConfigManager& configToUse)
        : Panel ("PROFILES"), config (configToUse)
    {
        nameLabel.setText ("PROFILE NAME", juce::dontSendNotification);
        nameLabel.setFont (monoFont (9.5f, true));
        nameLabel.setColour (juce::Label::textColourId, Palette::amberDim());
        addAndMakeVisible (nameLabel);

        nameEditor.setFont (monoFont (11.5f));
        nameEditor.setMultiLine (false);
        nameEditor.setReturnKeyStartsNewLine (false);
        nameEditor.setColour (juce::TextEditor::textColourId, Palette::amberBright());
        nameEditor.setColour (juce::TextEditor::backgroundColourId, Palette::bgPanelAlt());
        nameEditor.setTextToShowWhenEmpty ("type a name...", Palette::amberFaint());
        addAndMakeVisible (nameEditor);

        addAndMakeVisible (profileBox);
        addAndMakeVisible (saveButton);
        addAndMakeVisible (loadButton);
        addAndMakeVisible (deleteButton);

        saveButton.onClick   = [this] { doSave(); };
        loadButton.onClick   = [this] { doLoad(); };
        deleteButton.onClick = [this] { doDelete(); };

        profileBox.onChange = [this]
        {
            const int idx = profileBox.getSelectedItemIndex();
            if (idx >= 0)
                nameEditor.setText (config.getProfileName (idx), false);
        };

        refresh();
    }

    void ProfilesPanel::refresh()
    {
        const auto current = profileBox.getText();
        profileBox.clear (juce::dontSendNotification);

        const auto names = config.getProfileNames();
        for (int i = 0; i < names.size(); ++i)
            profileBox.addItem (names[i], i + 1);

        if (current.isNotEmpty() && names.contains (current))
            profileBox.setText (current, juce::dontSendNotification);
        else if (names.size() > 0)
            profileBox.setSelectedItemIndex (0, juce::dontSendNotification);
    }

    void ProfilesPanel::resized()
    {
        auto r = getContentBounds();

        auto nameRow = r.removeFromTop (30);
        nameLabel.setBounds (nameRow.removeFromTop (11));
        nameEditor.setBounds (nameRow);

        r.removeFromTop (5);

        listLabel.setVisible (false);
        auto listRow = r.removeFromTop (24);
        profileBox.setBounds (listRow);

        r.removeFromTop (6);

        auto buttons = r.removeFromTop (24);
        const int bw = (buttons.getWidth() - 12) / 3;
        saveButton.setBounds (buttons.removeFromLeft (bw));
        buttons.removeFromLeft (6);
        loadButton.setBounds (buttons.removeFromLeft (bw));
        buttons.removeFromLeft (6);
        deleteButton.setBounds (buttons);
    }

    void ProfilesPanel::doSave()
    {
        Profile p = onCapture ? onCapture() : Profile();

        auto name = nameEditor.getText().trim();
        if (name.isEmpty())
            name = "PROFILE " + juce::String (config.getNumProfiles() + 1);
        p.name = name;

        config.addOrUpdateProfile (p);
        config.saveProfiles();
        refresh();
        profileBox.setText (p.name, juce::dontSendNotification);
        nameEditor.setText (p.name, false);

        if (onLog != nullptr)
            onLog ("PROFILE SAVED: " + p.name);
    }

    void ProfilesPanel::doLoad()
    {
        const int idx = profileBox.getSelectedItemIndex();
        if (idx < 0)
            return;

        const auto p = config.getProfile (idx);
        if (onApply != nullptr)
            onApply (p);
        nameEditor.setText (p.name, false);

        if (onLog != nullptr)
            onLog ("PROFILE LOADED: " + p.name);
    }

    void ProfilesPanel::doDelete()
    {
        const int idx = profileBox.getSelectedItemIndex();
        if (idx < 0)
            return;

        const auto name = config.getProfileName (idx);
        config.removeProfile (idx);
        config.saveProfiles();
        refresh();
        nameEditor.clear();

        if (onLog != nullptr)
            onLog ("PROFILE DELETED: " + name);
    }
}
