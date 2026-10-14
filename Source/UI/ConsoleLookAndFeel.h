#pragma once

#include "Common.h"

namespace vp
{
    // Amber monochrome console LookAndFeel. Mouse / touch oriented controls only.
    class ConsoleLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        ConsoleLookAndFeel();

        juce::Font getLabelFont (juce::Label&) override;
        juce::Font getComboBoxFont (juce::ComboBox&) override;
        juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
        juce::Font getPopupMenuFont() override;
        juce::Font getSliderPopupFont (juce::Slider&) override;

        void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                                   bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        void drawButtonText (juce::Graphics&, juce::TextButton&,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                           int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox&) override;

        void positionComboBoxText (juce::ComboBox&, juce::Label&) override;

        void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               juce::Slider::SliderStyle, juce::Slider&) override;

        void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

        void drawScrollbar (juce::Graphics&, juce::ScrollBar&, int x, int y, int width, int height,
                            bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                            bool isMouseOver, bool isMouseDown) override;

        void fillTextEditorBackground (juce::Graphics&, int width, int height, juce::TextEditor&) override;
        void drawTextEditorOutline (juce::Graphics&, int width, int height, juce::TextEditor&) override;

        void drawPopupMenuBackground (juce::Graphics&, int width, int height) override;
        void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area,
                                bool isSeparator, bool isActive, bool isHighlighted,
                                bool isTicked, bool hasSubMenu, const juce::String& text,
                                const juce::String& shortcutKeyText, const juce::Drawable* icon,
                                const juce::Colour* textColour) override;

        juce::Button* createDocumentWindowButton (int buttonType) override;

        void drawDocumentWindowTitleBar (juce::DocumentWindow&, juce::Graphics&,
                                         int w, int h, int titleSpaceX, int titleSpaceW,
                                         const juce::Image* icon, bool drawTitleTextOnLeft) override;

        // Re-applies the active theme colours (call after ThemeManager::setCurrentIndex).
        void applyTheme();

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleLookAndFeel)
    };
}
