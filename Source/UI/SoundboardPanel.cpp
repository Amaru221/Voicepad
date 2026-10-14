#include "SoundboardPanel.h"

namespace vp
{
    //==============================================================================
    void SoundboardPanel::FavoritesBar::setIndices (std::vector<int> newIndices)
    {
        indices = std::move (newIndices);
        rebuild();
        repaint();
    }

    void SoundboardPanel::FavoritesBar::resized()
    {
        rebuild();
    }

    void SoundboardPanel::FavoritesBar::rebuild()
    {
        rects.clear();
        if (board == nullptr || indices.empty() || getWidth() <= 0)
            return;

        const int chipH = 20;
        const int gap = 4;
        const int x0 = 0;
        int x = x0;
        int y = 0;

        for (int idx : indices)
        {
            const auto name = board->getSlotName (idx);
            const int w = juce::jlimit (48, 190, (int) monoFont (10.5f, true).getStringWidth (name) + 22);

            if (x + w > getWidth() && x > x0)
            {
                x = x0;
                y += chipH + gap;
            }

            rects.push_back ({ x, y, w, chipH });
            x += w + gap;
        }
    }

    void SoundboardPanel::FavoritesBar::paint (juce::Graphics& g)
    {
        if (board == nullptr)
            return;

        for (int i = 0; i < (int) indices.size() && i < (int) rects.size(); ++i)
        {
            const int idx = indices[(size_t) i];
            auto r = rects[(size_t) i].toFloat();
            const bool playing = board->isPlaying (idx);
            const bool hot = (i == hover);

            g.setColour (playing ? Palette::amberDim().withAlpha (0.6f)
                                 : (hot ? Palette::amberGhost().brighter (0.15f) : Palette::bgPanelAlt()));
            g.fillRect (r);
            g.setColour (playing ? Palette::amberBright() : (hot ? Palette::amber() : Palette::amberDim()));
            g.drawRect (r, 1.0f);

            g.setColour (playing ? Palette::amberBright() : Palette::amber());
            g.setFont (monoFont (10.5f, true));
            g.drawFittedText (board->getSlotName (idx), r.toNearestInt().reduced (6, 0), juce::Justification::centred, 1);

            if (playing)
            {
                const float p = board->getProgress (idx);
                g.setColour (Palette::amberBright());
                g.fillRect (r.getX(), r.getBottom() - 2.0f, r.getWidth() * p, 2.0f);
            }
        }
    }

    void SoundboardPanel::FavoritesBar::mouseDown (const juce::MouseEvent& e)
    {
        for (int i = 0; i < (int) rects.size(); ++i)
            if (rects[(size_t) i].contains (e.getPosition()) && i < (int) indices.size())
            {
                if (onPlay != nullptr)
                    onPlay (indices[(size_t) i]);
                return;
            }
    }

    void SoundboardPanel::FavoritesBar::mouseMove (const juce::MouseEvent& e)
    {
        int newHover = -1;
        for (int i = 0; i < (int) rects.size(); ++i)
            if (rects[(size_t) i].contains (e.getPosition()))
                newHover = i;

        if (newHover != hover)
        {
            hover = newHover;
            repaint();
        }
    }

    void SoundboardPanel::FavoritesBar::mouseExit (const juce::MouseEvent&)
    {
        if (hover != -1)
        {
            hover = -1;
            repaint();
        }
    }

    //==============================================================================
    SoundboardPanel::SoundboardPanel (Soundboard& boardToUse)
        : Panel ("SOUNDBOARD MATRIX"), soundboard (boardToUse)
    {
        favoritesBar.board = &soundboard;
        favoritesBar.onPlay = [this] (int slot) { playSlot (slot); };
        addAndMakeVisible (favoritesBar);

        searchBox.setFont (monoFont (11.5f));
        searchBox.setMultiLine (false);
        searchBox.setReturnKeyStartsNewLine (false);
        searchBox.setTextToShowWhenEmpty ("search by name...", Palette::amberFaint());
        searchBox.setColour (juce::TextEditor::textColourId, Palette::amberBright());
        searchBox.setColour (juce::TextEditor::backgroundColourId, Palette::bgPanelAlt());
        searchBox.onTextChange = [this] { refreshAll(); };
        addAndMakeVisible (searchBox);

        clearButton.setColour (juce::TextButton::buttonColourId, Palette::bgPanelAlt());
        clearButton.onClick = [this] { searchBox.clear(); refreshAll(); searchBox.grabKeyboardFocus(); };
        addAndMakeVisible (clearButton);

        importButton.onClick = [this] { importSounds(); };
        addAndMakeVisible (importButton);

        stopButton.onClick = [this] { soundboard.stopAll(); };
        addAndMakeVisible (stopButton);

        list.setModel (this);
        list.setRowHeight (20);
        list.setColour (juce::ListBox::backgroundColourId, Palette::bgPanelAlt());
        list.setColour (juce::ListBox::outlineColourId, Palette::amberFaint());
        list.setOutlineThickness (1);
        addAndMakeVisible (list);

        lastDir = juce::File::getSpecialLocation (juce::File::userHomeDirectory);
        refreshAll();
        startTimerHz (20);
    }

    SoundboardPanel::~SoundboardPanel()
    {
        list.setModel (nullptr);
    }

    void SoundboardPanel::saveLibrary()
    {
        if (onLibraryChanged != nullptr)
            onLibraryChanged();
    }

    void SoundboardPanel::refreshAll()
    {
        const auto query = searchBox.getText().trim();

        filtered.clear();
        for (int i = 0; i < soundboard.getNumSlots(); ++i)
            if (query.isEmpty() || soundboard.getSlotName (i).containsIgnoreCase (query))
                filtered.push_back (i);

        std::vector<int> favs;
        for (int i = 0; i < soundboard.getNumSlots(); ++i)
            if (soundboard.isFavorite (i))
                favs.push_back (i);

        favoritesBar.setIndices (favs);

        list.updateContent();
        list.repaint();
        repaint();
    }

    int SoundboardPanel::slotForRow (int row) const
    {
        if (juce::isPositiveAndBelow (row, (int) filtered.size()))
            return filtered[(size_t) row];
        return -1;
    }

    void SoundboardPanel::playSlot (int slot)
    {
        if (slot < 0)
            return;
        soundboard.trigger (slot);
        if (onTriggered != nullptr)
            onTriggered (slot);
    }

    void SoundboardPanel::importSounds()
    {
        chooser = std::make_unique<juce::FileChooser> ("Importar sonidos",
                                                       lastDir,
                                                       "*.wav;*.aiff;*.aif;*.flac;*.ogg;*.mp3;*.m4a;*.wma;*.aac");

        chooser->launchAsync (juce::FileBrowserComponent::openMode
                                  | juce::FileBrowserComponent::canSelectFiles
                                  | juce::FileBrowserComponent::canSelectMultipleItems,
                              [this] (const juce::FileChooser& fc)
        {
            const auto results = fc.getResults();
            if (results.isEmpty())
                return;

            for (const auto& f : results)
            {
                soundboard.addUserSound (f);
                lastDir = f.getParentDirectory();
            }

            refreshAll();
            saveLibrary();
        });
    }

    void SoundboardPanel::showRowMenu (int row)
    {
        const int slot = slotForRow (row);
        if (slot < 0)
            return;

        juce::PopupMenu menu;
        menu.addItem (1, "PLAY");
        menu.addItem (2, soundboard.isFavorite (slot) ? "REMOVE FROM FAVOURITES" : "ADD TO FAVOURITES");
        menu.addSeparator();
        menu.addItem (3, "DELETE SOUND", soundboard.isUserSlot (slot));

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&list),
                            [this, slot] (int result)
        {
            if (result == 1)
                playSlot (slot);
            else if (result == 2)
            {
                soundboard.toggleFavorite (slot);
                refreshAll();
                saveLibrary();
            }
            else if (result == 3)
                deleteSlot (slot);
        });
    }

    void SoundboardPanel::deleteSlot (int slot)
    {
        soundboard.removeSlot (slot);
        refreshAll();
        saveLibrary();
    }

    void SoundboardPanel::resized()
    {
        auto r = getContentBounds();

        favLabelArea = r.removeFromTop (12);
        const int favH = juce::jlimit (40, 96, r.getHeight() / 3);
        favoritesBar.setBounds (r.removeFromTop (favH));
        r.removeFromTop (4);

        auto searchRow = r.removeFromTop (24);
        clearButton.setBounds (searchRow.removeFromRight (24));
        searchRow.removeFromRight (2);
        searchBox.setBounds (searchRow);
        r.removeFromTop (4);

        auto headRow = r.removeFromTop (22);
        importButton.setBounds (headRow.removeFromRight (78));
        headRow.removeFromRight (4);
        stopButton.setBounds (headRow.removeFromRight (58));
        libLabelArea = headRow;

        list.setBounds (r);
    }

    void SoundboardPanel::paintPanelContent (juce::Graphics& g)
    {
        g.setFont (monoFont (9.0f, true));
        g.setColour (Palette::amberDim());
        g.drawFittedText ("FAVOURITES  //  CLICK TO PLAY", favLabelArea, juce::Justification::centredLeft, 1);

        g.setColour (Palette::amberDim());
        g.drawFittedText ("LIBRARY (" + juce::String (soundboard.getNumSlots()) + ")",
                          libLabelArea, juce::Justification::centredLeft, 1);
    }

    void SoundboardPanel::timerCallback()
    {
        list.repaint();
        favoritesBar.repaint();
    }

    int SoundboardPanel::getNumRows()
    {
        return (int) filtered.size();
    }

    void SoundboardPanel::paintListBoxItem (int row, juce::Graphics& g, int width, int height, bool rowIsSelected)
    {
        const int slot = slotForRow (row);
        if (slot < 0)
            return;

        const bool playing = soundboard.isPlaying (slot);
        const bool fav = soundboard.isFavorite (slot);
        const bool user = soundboard.isUserSlot (slot);

        auto r = juce::Rectangle<int> (0, 0, width, height);

        if (rowIsSelected)
        {
            g.setColour (Palette::amberDim());
            g.fillRect (r);
        }
        else if (row % 2 == 1)
        {
            g.setColour (Palette::amberGhost().withAlpha (0.3f));
            g.fillRect (r);
        }

        // favourite star
        g.setColour (fav ? Palette::amberBright() : Palette::amberFaint());
        g.setFont (monoFont (11.0f, true));
        g.drawText (fav ? "*" : ".", r.removeFromLeft (16), juce::Justification::centred);

        // source tag
        auto tagArea = r.removeFromRight (40);
        g.setColour (user ? Palette::amberMid() : Palette::amberDim());
        g.setFont (monoFont (8.5f));
        g.drawText (user ? "USER" : "KIT", tagArea, juce::Justification::centred);

        g.setColour (playing ? Palette::amberBright() : Palette::amber());
        g.setFont (monoFont (11.0f, playing));
        g.drawFittedText (soundboard.getSlotName (slot), r.reduced (4, 0), juce::Justification::centredLeft, 1);

        if (playing)
        {
            const float p = soundboard.getProgress (slot);
            g.setColour (Palette::amberDim());
            g.fillRect (0, height - 2, width, 2);
            g.setColour (Palette::amberBright());
            g.fillRect (0, height - 2, (int) (width * p), 2);
        }
    }

    void SoundboardPanel::listBoxItemClicked (int row, const juce::MouseEvent& e)
    {
        if (e.mods.isPopupMenu())
            showRowMenu (row);
        else
            playSlot (slotForRow (row));
    }
}
