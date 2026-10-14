#pragma once

#include <memory>
#include <vector>
#include "Panel.h"
#include "Soundboard/Soundboard.h"

namespace vp
{
    // Soundboard: favourites strip, search box (with clear), full library list,
    // and an import button for user sounds (WAV/AIFF/FLAC/Ogg/MP3).
    class SoundboardPanel : public Panel, private juce::Timer, private juce::ListBoxModel
    {
    public:
        explicit SoundboardPanel (Soundboard& boardToUse);
        ~SoundboardPanel() override;

        void resized() override;
        void paintPanelContent (juce::Graphics&) override;

        std::function<void()> onLibraryChanged;   // import / delete / favourite changed
        std::function<void (int)> onTriggered;

        void refreshAll();
        void saveLibrary();

    private:
        struct FavoritesBar : juce::Component
        {
            Soundboard* board = nullptr;
            std::vector<int> indices;
            std::function<void (int)> onPlay;

            void setIndices (std::vector<int> newIndices);
            void resized() override;
            void paint (juce::Graphics&) override;
            void mouseDown (const juce::MouseEvent&) override;
            void mouseMove (const juce::MouseEvent&) override;
            void mouseExit (const juce::MouseEvent&) override;

            std::vector<juce::Rectangle<int>> rects;
            int hover = -1;
            void rebuild();
        };

        void timerCallback() override;
        void playSlot (int slot);
        void importSounds();
        void showRowMenu (int row);
        void deleteSlot (int slot);
        int  slotForRow (int row) const;

        // ListBoxModel
        int getNumRows() override;
        void paintListBoxItem (int row, juce::Graphics&, int width, int height, bool rowIsSelected) override;
        void listBoxItemClicked (int row, const juce::MouseEvent&) override;

        Soundboard& soundboard;
        FavoritesBar favoritesBar;
        juce::TextEditor searchBox;
        juce::TextButton clearButton { "X" };
        juce::TextButton importButton { "IMPORT" };
        juce::TextButton stopButton { "STOP" };
        juce::ListBox list;
        std::unique_ptr<juce::FileChooser> chooser;

        std::vector<int> filtered;
        juce::Rectangle<int> favLabelArea, libLabelArea;
        juce::File lastDir;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SoundboardPanel)
    };
}
