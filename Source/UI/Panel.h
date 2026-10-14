#pragma once

#include "Common.h"

namespace vp
{
    // Base tiled panel: thin amber border, header bar with title, optional simple tabs,
    // and optional per-section RESET / SAVE buttons (mouse / touch only).
    class Panel : public juce::Component
    {
    public:
        explicit Panel (juce::String panelTitle, bool withTabs = false);

        void setTitle (juce::String newTitle);
        void setTabs (const juce::StringArray& tabNames);
        void setActiveTab (int index);
        int getActiveTab() const noexcept { return activeTab; }

        // Enable the little header buttons for this section.
        void setSectionActions (bool hasReset, bool hasSave);

        std::function<void (int)> onTabChanged;
        std::function<void()> onResetSection;
        std::function<void()> onSaveSection;

        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseMove (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;

        juce::Rectangle<int> getContentBounds() const;

        static constexpr int headerHeight = 22;

    protected:
        virtual void paintPanelBackground (juce::Graphics& g);
        virtual void paintPanelContent (juce::Graphics& g);
        virtual void resizedPanel() {}

        juce::String title;
        juce::StringArray tabs;
        int activeTab = 0;

    private:
        juce::Rectangle<int> getHeaderBounds() const;
        juce::Rectangle<int> getTabBounds (int index) const;
        juce::Rectangle<int> getActionButtonBounds (int buttonIndex) const; // 0 = reset, 1 = save

        bool showReset = false;
        bool showSave = false;
        int hoverButton = -1;
    };
}
