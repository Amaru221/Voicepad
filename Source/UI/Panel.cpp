#include "Panel.h"

namespace vp
{
    Panel::Panel (juce::String panelTitle, bool withTabs)
        : title (std::move (panelTitle))
    {
        if (withTabs)
            tabs.add ("TAB");
    }

    void Panel::setTitle (juce::String newTitle)
    {
        title = std::move (newTitle);
        repaint();
    }

    void Panel::setTabs (const juce::StringArray& tabNames)
    {
        tabs = tabNames;
        activeTab = 0;
        repaint();
    }

    void Panel::setActiveTab (int index)
    {
        if (juce::isPositiveAndBelow (index, tabs.size()))
        {
            activeTab = index;
            repaint();
            if (onTabChanged != nullptr)
                onTabChanged (activeTab);
        }
    }

    void Panel::setSectionActions (bool hasReset, bool hasSave)
    {
        showReset = hasReset;
        showSave = hasSave;
        repaint();
    }

    juce::Rectangle<int> Panel::getHeaderBounds() const
    {
        return getLocalBounds().removeFromTop (headerHeight);
    }

    juce::Rectangle<int> Panel::getContentBounds() const
    {
        return getLocalBounds().withTrimmedTop (headerHeight).reduced (6, 5);
    }

    juce::Rectangle<int> Panel::getTabBounds (int index) const
    {
        auto header = getHeaderBounds().reduced (1, 1);
        int tabWidth = 74;
        int x = header.getX() + 6;
        return { x + index * (tabWidth + 2), header.getY(), tabWidth, header.getHeight() };
    }

    juce::Rectangle<int> Panel::getActionButtonBounds (int buttonIndex) const
    {
        auto header = getHeaderBounds();
        const int bw = 42;
        const int bh = 15;
        const int gap = 4;
        const int y = header.getY() + (header.getHeight() - bh) / 2;

        int right = header.getRight() - 5;

        if (showSave && buttonIndex == 1)
            return { right - bw, y, bw, bh };

        if (showSave)
            right -= (bw + gap);

        if (showReset && buttonIndex == 0)
            return { right - bw, y, bw, bh };

        return {};
    }

    void Panel::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds();

        g.setColour (Palette::bgPanel());
        g.fillRect (bounds);

        paintPanelBackground (g);

        auto header = getHeaderBounds();
        g.setColour (Palette::amberGhost());
        g.fillRect (header);
        g.setColour (Palette::amberDim());
        g.fillRect (header.getX(), header.getBottom() - 1, header.getWidth(), 1);

        auto titleArea = header.reduced (8, 0);

        if (tabs.size() > 0)
        {
            for (int i = 0; i < tabs.size(); ++i)
            {
                auto tb = getTabBounds (i);
                bool active = (i == activeTab);

                g.setColour (active ? Palette::amberGhost().brighter (0.10f) : Palette::bgPanel());
                g.fillRect (tb);

                g.setColour (active ? Palette::amber() : Palette::amberFaint());
                g.drawRect (tb, 1);

                g.setFont (monoFont (11.5f, active));
                g.setColour (active ? Palette::amberBright() : Palette::amberDim());
                g.drawFittedText (tabs[i], tb, juce::Justification::centred, 1);
            }

            titleArea = titleArea.withTrimmedLeft (6 + tabs.size() * 76 + 8);
        }

        titleArea = titleArea.withTrimmedRight (showSave || showReset ? 96 : 4);

        if (showSave || showReset)
        {
            titleArea = titleArea.withTrimmedLeft (0);
            g.setFont (monoFont (11.5f, true));
            g.setColour (Palette::amber());
            g.drawFittedText (title, titleArea, juce::Justification::centredLeft, 1);

            auto paintButton = [&] (int index, const juce::String& label, bool enabled)
            {
                if (! enabled)
                    return;
                auto r = getActionButtonBounds (index).toFloat().reduced (0.5f);
                const bool hot = (hoverButton == index);

                g.setColour (hot ? Palette::amberGhost().brighter (0.18f) : Palette::bgPanelAlt());
                g.fillRect (r);
                g.setColour (hot ? Palette::amberBright() : Palette::amberDim());
                g.drawRect (r, 1.0f);
                g.setColour (hot ? Palette::amberBright() : Palette::amberMid());
                g.setFont (monoFont (8.5f, true));
                g.drawFittedText (label, r.toNearestInt(), juce::Justification::centred, 1);
            };

            paintButton (0, "RESET", showReset);
            paintButton (1, "SAVE", showSave);
        }
        else
        {
            g.setFont (monoFont (12.5f, true));
            g.setColour (Palette::amber());
            g.drawFittedText (title.toUpperCase(), titleArea, juce::Justification::centredLeft, 1);

            g.setColour (Palette::amberDim());
            g.drawFittedText ("::", header.reduced (8, 0), juce::Justification::centredRight, 1);
        }

        paintPanelContent (g);

        g.setColour (Palette::amberDim());
        g.drawRect (bounds, 1);
    }

    void Panel::resized()
    {
        resizedPanel();
    }

    void Panel::paintPanelBackground (juce::Graphics&) {}
    void Panel::paintPanelContent (juce::Graphics&) {}

    void Panel::mouseDown (const juce::MouseEvent& e)
    {
        if (getHeaderBounds().contains (e.getPosition()))
        {
            if (showReset && getActionButtonBounds (0).contains (e.getPosition()))
            {
                if (onResetSection != nullptr)
                    onResetSection();
                return;
            }

            if (showSave && getActionButtonBounds (1).contains (e.getPosition()))
            {
                if (onSaveSection != nullptr)
                    onSaveSection();
                return;
            }

            if (tabs.size() > 0)
            {
                for (int i = 0; i < tabs.size(); ++i)
                {
                    if (getTabBounds (i).contains (e.getPosition()))
                    {
                        setActiveTab (i);
                        return;
                    }
                }
            }
        }
    }

    void Panel::mouseMove (const juce::MouseEvent& e)
    {
        int newHover = -1;
        if (showReset && getActionButtonBounds (0).contains (e.getPosition()))
            newHover = 0;
        else if (showSave && getActionButtonBounds (1).contains (e.getPosition()))
            newHover = 1;

        if (newHover != hoverButton)
        {
            hoverButton = newHover;
            repaint();
        }
    }

    void Panel::mouseExit (const juce::MouseEvent&)
    {
        if (hoverButton != -1)
        {
            hoverButton = -1;
            repaint();
        }
    }
}
