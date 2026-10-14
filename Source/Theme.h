#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace vp
{
    // A selectable colour scheme for the whole console.
    struct ThemeColors
    {
        juce::String name;
        juce::Colour bg, bgPanel, bgPanelAlt, bgScreen;
        juce::Colour primary, primaryBright, primaryMid, primaryDim, primaryFaint, primaryGhost;
        juce::Colour alert, ok, shadow;
    };

    class ThemeManager
    {
    public:
        static const juce::Array<ThemeColors>& themes();
        static int getNumThemes();
        static juce::StringArray names();

        static int getCurrentIndex();
        static const ThemeColors& current();
        static void setCurrentIndex (int index);
    };

    // Compatibility accessors used across the UI. They always read the live theme.
    struct Palette
    {
        static juce::Colour bg()           { return ThemeManager::current().bg; }
        static juce::Colour bgPanel()      { return ThemeManager::current().bgPanel; }
        static juce::Colour bgPanelAlt()   { return ThemeManager::current().bgPanelAlt; }
        static juce::Colour bgScreen()     { return ThemeManager::current().bgScreen; }

        static juce::Colour amber()        { return ThemeManager::current().primary; }
        static juce::Colour amberBright()  { return ThemeManager::current().primaryBright; }
        static juce::Colour amberMid()     { return ThemeManager::current().primaryMid; }
        static juce::Colour amberDim()     { return ThemeManager::current().primaryDim; }
        static juce::Colour amberFaint()   { return ThemeManager::current().primaryFaint; }
        static juce::Colour amberGhost()   { return ThemeManager::current().primaryGhost; }

        static juce::Colour alert()        { return ThemeManager::current().alert; }
        static juce::Colour ok()           { return ThemeManager::current().ok; }
        static juce::Colour shadow()       { return ThemeManager::current().shadow; }
    };

    inline juce::Font monoFont (float height, bool bold = false)
    {
        auto opts = juce::FontOptions()
                        .withName ("Consolas")
                        .withHeight (height);
        if (bold)
            opts = opts.withStyle ("Bold");
        return juce::Font (opts);
    }
}
