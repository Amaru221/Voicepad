#include "ManualPanel.h"

namespace vp
{
    ManualPanel::ManualPanel()
        : Panel ("OPERATOR MANUAL // MOUSE + TOUCH")
    {
    }

    void ManualPanel::paintPanelContent (juce::Graphics& g)
    {
        auto area = getContentBounds();

        juce::AttributedString as;
        as.setJustification (juce::Justification::topLeft);

        auto heading = [&] (const juce::String& t)
        {
            as.append ("\n" + t + "\n", monoFont (10.0f, true), Palette::amberBright());
        };
        auto line = [&] (const juce::String& t)
        {
            as.append ("  " + t + "\n", monoFont (9.5f), Palette::amberMid());
        };

        as.append ("POINTER CONTROL REFERENCE\n", monoFont (10.5f, true), Palette::amber());

        heading ("SIGNAL MAP");
        line ("Drag the scope to pan the view window.");
        line ("Double-click the scope to cycle zoom.");
        line ("Hover a numbered marker for its readout.");

        heading ("DATA TABLES");
        line ("Click a column header to sort by it.");
        line ("Click a header again to reverse order.");
        line ("Scroll the list with the mouse wheel.");

        heading ("PRESETS & EFFECTS");
        line ("Click a preset pad to apply it.");
        line ("Drag sliders to shape the voice.");
        line ("Double-click a slider to recentre it.");

        heading ("SOUNDBOARD");
        line ("Click a pad to fire its clip.");
        line ("The bar under a pad shows progress.");

        juce::TextLayout layout;
        layout.createLayout (as, (float) area.getWidth());
        layout.draw (g, area.toFloat());
    }
}
