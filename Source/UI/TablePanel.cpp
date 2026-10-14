#include "TablePanel.h"

namespace vp
{
    TablePanel::TablePanel (juce::String title, const std::vector<Column>& columnsToUse)
        : Panel (std::move (title)), columns (columnsToUse)
    {
    }

    int TablePanel::totalWidth() const
    {
        int w = 0;
        for (const auto& c : columns)
            w += c.width;
        return w;
    }

    juce::Rectangle<int> TablePanel::getTableArea() const
    {
        return getContentBounds();
    }

    juce::Rectangle<int> TablePanel::getHeaderRow() const
    {
        return getTableArea().removeFromTop (getFieldHeight());
    }

    int TablePanel::columnAt (int x) const
    {
        int cx = getTableArea().getX();
        for (int i = 0; i < (int) columns.size(); ++i)
        {
            if (x >= cx && x < cx + columns[(size_t) i].width)
                return i;
            cx += columns[(size_t) i].width;
        }
        return -1;
    }

    void TablePanel::setRows (juce::Array<juce::StringArray> newRows)
    {
        rows = std::move (newRows);
        resort();
        repaint();
    }

    void TablePanel::resetView()
    {
        sortColumn = -1;
        sortAscending = true;
        selectedRow = -1;
        scrollOffset = 0;
        hoverColumn = -1;
        repaint();
    }

    void TablePanel::resort()
    {
        if (sortColumn < 0)
            return;

        const int col = sortColumn;
        const bool asc = sortAscending;

        struct Cmp
        {
            int col;
            bool asc;
            bool operator() (const juce::StringArray& a, const juce::StringArray& b) const
            {
                auto av = col < a.size() ? a[col] : juce::String();
                auto bv = col < b.size() ? b[col] : juce::String();

                const double an = av.getDoubleValue();
                const double bn = bv.getDoubleValue();
                const bool aNum = av.containsAnyOf ("0123456789") && ! av.containsOnly ("0123456789:.-+ ");
                const bool bNum = bv.containsAnyOf ("0123456789") && ! bv.containsOnly ("0123456789:.-+ ");

                bool less;
                if (aNum && bNum)
                    less = an < bn;
                else
                    less = av.compareIgnoreCase (bv) < 0;

                return asc ? less : ! less;
            }
        };

        std::sort (rows.begin(), rows.end(), Cmp { col, asc });
    }

    void TablePanel::paintPanelContent (juce::Graphics& g)
    {
        auto area = getTableArea();
        if (area.isEmpty())
            return;

        g.saveState();
        g.reduceClipRegion (area);

        auto header = getHeaderRow();

        // header background
        g.setColour (Palette::amberGhost());
        g.fillRect (header);

        // header titles
        int x = area.getX();
        for (int i = 0; i < (int) columns.size(); ++i)
        {
            const auto& c = columns[(size_t) i];
            juce::Rectangle<int> cell (x, header.getY(), c.width, header.getHeight());

            g.setColour (i == hoverColumn ? Palette::amberBright() : Palette::amber());
            g.setFont (monoFont (11.0f, true));
            g.drawFittedText (c.header, cell.reduced (5, 0), juce::Justification::centredLeft, 1);

            if (i == sortColumn)
            {
                g.setColour (Palette::amberBright());
                g.drawText (sortAscending ? "^" : "v", cell.removeFromRight (12), juce::Justification::centred);
            }

            g.setColour (Palette::amberFaint());
            g.drawLine ((float) (x + c.width), (float) header.getY(), (float) (x + c.width), (float) header.getBottom());
            x += c.width;
        }

        g.setColour (Palette::amberDim());
        g.drawLine ((float) area.getX(), (float) header.getBottom(), (float) area.getRight(), (float) header.getBottom());

        // rows
        auto listArea = area.withTrimmedTop (getFieldHeight());
        const int rowH = getFieldHeight();
        int y = listArea.getY() - scrollOffset;

        for (int r = 0; r < rows.size(); ++r)
        {
            juce::Rectangle<int> rowRect (area.getX(), y, totalWidth(), rowH);

            if (rowRect.getBottom() >= listArea.getY() && rowRect.getY() <= listArea.getBottom())
            {
                if (r == selectedRow)
                {
                    g.setColour (Palette::amberDim());
                    g.fillRect (rowRect);
                }
                else if (r % 2 == 1)
                {
                    g.setColour (Palette::amberGhost().withAlpha (0.35f));
                    g.fillRect (rowRect);
                }

                const auto& row = rows[r];
                int cx = area.getX();
                for (int i = 0; i < (int) columns.size(); ++i)
                {
                    const auto& c = columns[(size_t) i];
                    juce::Rectangle<int> cell (cx, y, c.width, rowH);

                    const bool alert = (i < row.size() && row[i].startsWith ("!"));
                    g.setColour (r == selectedRow ? Palette::bg()
                                                  : (alert ? Palette::alert() : Palette::amberMid()));
                    g.setFont (monoFont (10.5f));
                    g.drawFittedText (i < row.size() ? row[i].trimCharactersAtStart ("!") : juce::String(),
                                      cell.reduced (5, 0), juce::Justification::centredLeft, 1);
                    cx += c.width;
                }
            }

            y += rowH;
            if (y > listArea.getBottom() + rowH)
                break;
        }

        g.restoreState();

        // minimal scrollbar
        const int total = rows.size() * rowH;
        if (total > listArea.getHeight())
        {
            const float thumbH = juce::jmax (18.0f, (float) listArea.getHeight() * (float) listArea.getHeight() / (float) total);
            const float maxScroll = (float) (total - listArea.getHeight());
            const float t = maxScroll > 0.0f ? (float) scrollOffset / maxScroll : 0.0f;
            const float yPos = (float) listArea.getY() + t * ((float) listArea.getHeight() - thumbH);

            g.setColour (Palette::amberDim());
            g.fillRect ((float) area.getRight() - 3.0f, yPos, 2.0f, thumbH);
        }
    }

    void TablePanel::mouseDown (const juce::MouseEvent& e)
    {
        Panel::mouseDown (e);

        auto area = getTableArea();
        if (! area.contains (e.getPosition()))
            return;

        if (getHeaderRow().contains (e.getPosition()))
        {
            const int col = columnAt (e.x);
            if (col >= 0)
            {
                if (col == sortColumn)
                    sortAscending = ! sortAscending;
                else
                {
                    sortColumn = col;
                    sortAscending = true;
                }
                resort();
                repaint();
            }
            return;
        }

        const int rowH = getFieldHeight();
        const int idx = (e.y - (area.getY() + rowH) + scrollOffset) / rowH;
        if (juce::isPositiveAndBelow (idx, rows.size()))
        {
            selectedRow = idx;
            if (onRowSelected != nullptr)
                onRowSelected (idx);
            repaint();
        }
    }

    void TablePanel::mouseMove (const juce::MouseEvent& e)
    {
        Panel::mouseMove (e);

        const int col = getHeaderRow().contains (e.getPosition()) ? columnAt (e.x) : -1;
        if (col != hoverColumn)
        {
            hoverColumn = col;
            repaint();
        }
    }

    void TablePanel::mouseExit (const juce::MouseEvent& e)
    {
        Panel::mouseExit (e);

        if (hoverColumn != -1)
        {
            hoverColumn = -1;
            repaint();
        }
    }

    void TablePanel::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
    {
        const int rowH = getFieldHeight();
        const int listHeight = getTableArea().getHeight() - rowH;
        const int total = rows.size() * rowH;
        const int maxScroll = juce::jmax (0, total - listHeight);

        scrollOffset = juce::jlimit (0, maxScroll, scrollOffset - (int) (wheel.deltaY * 120.0f));
        repaint();
    }
}
