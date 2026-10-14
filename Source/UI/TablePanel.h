#pragma once

#include "Panel.h"

namespace vp
{
    // Dense sortable data table. Sorting by clicking a column header (mouse only).
    class TablePanel : public Panel
    {
    public:
        struct Column
        {
            juce::String header;
            int width = 80;
        };

        TablePanel (juce::String title, const std::vector<Column>& columnsToUse);

        void setRows (juce::Array<juce::StringArray> newRows);

        std::function<void (int rowIndex)> onRowSelected;

        void resetView();

        void paintPanelContent (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseMove (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails&) override;

    private:
        int getFieldHeight() const noexcept { return 17; }
        int totalWidth() const;
        juce::Rectangle<int> getTableArea() const;
        juce::Rectangle<int> getHeaderRow() const;
        int columnAt (int x) const;
        void resort();

        std::vector<Column> columns;
        juce::Array<juce::StringArray> rows;
        int sortColumn = -1;
        bool sortAscending = true;
        int scrollOffset = 0;
        int selectedRow = -1;
        int hoverColumn = -1;
    };
}
