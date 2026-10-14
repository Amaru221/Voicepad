#include "Theme.h"

namespace
{
    vp::ThemeColors makeTheme (const char* name, juce::Colour primary, juce::Colour bg)
    {
        vp::ThemeColors t;
        t.name = name;

        t.primary       = primary;
        t.primaryBright = primary.brighter (0.55f);
        t.primaryMid    = primary.withMultipliedBrightness (0.82f);
        t.primaryDim    = primary.withMultipliedBrightness (0.50f);
        t.primaryFaint  = primary.withMultipliedBrightness (0.28f);
        t.primaryGhost  = primary.withMultipliedBrightness (0.15f);

        t.bg         = bg;
        t.bgPanel    = bg.brighter (0.05f);
        t.bgPanelAlt = bg.brighter (0.10f);
        t.bgScreen   = bg.withMultipliedBrightness (0.55f);

        t.alert  = juce::Colour (0xffe0502a);
        t.ok     = juce::Colour (0xffb6d132);
        t.shadow = juce::Colour (0xcc000000);
        return t;
    }

    int& currentThemeIndex()
    {
        static int index = 0;
        return index;
    }
}

namespace vp
{
    const juce::Array<ThemeColors>& ThemeManager::themes()
    {
        static juce::Array<ThemeColors> list
        {
            makeTheme ("AMBER",    juce::Colour (0xffffb000), juce::Colour (0xff080604)),
            makeTheme ("PHOSPHOR", juce::Colour (0xff6dff5a), juce::Colour (0xff03100a)),
            makeTheme ("ICE",      juce::Colour (0xff3fd6ff), juce::Colour (0xff02080e)),
            makeTheme ("EMBER",    juce::Colour (0xffff6a26), juce::Colour (0xff0d0402)),
            makeTheme ("SAPPHIRE", juce::Colour (0xff5a90ff), juce::Colour (0xff03060f)),
            makeTheme ("VIOLET",   juce::Colour (0xffc46bff), juce::Colour (0xff0a0410)),
            makeTheme ("MONO",     juce::Colour (0xffe8dfc8), juce::Colour (0xff0b0b0a))
        };
        return list;
    }

    int ThemeManager::getNumThemes()
    {
        return themes().size();
    }

    juce::StringArray ThemeManager::names()
    {
        juce::StringArray arr;
        for (const auto& t : themes())
            arr.add (t.name);
        return arr;
    }

    int ThemeManager::getCurrentIndex()
    {
        return currentThemeIndex();
    }

    const ThemeColors& ThemeManager::current()
    {
        return themes().getReference (juce::jlimit (0, themes().size() - 1, currentThemeIndex()));
    }

    void ThemeManager::setCurrentIndex (int index)
    {
        currentThemeIndex() = juce::jlimit (0, themes().size() - 1, index);
    }
}
