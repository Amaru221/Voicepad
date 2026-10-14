#include "ConsoleLookAndFeel.h"

namespace vp
{
    namespace
    {
        // Amber glyph window buttons (minimise / maximise / close) for the integrated title bar.
        class WindowGlyphButton : public juce::Button
        {
        public:
            explicit WindowGlyphButton (int typeToUse)
                : juce::Button ("wnd"), type (typeToUse)
            {
            }

            void paintButton (juce::Graphics& g, bool over, bool down) override
            {
                auto r = getLocalBounds().toFloat().reduced (2.0f);
                const bool isClose = (type == juce::DocumentWindow::closeButton);

                if (over)
                {
                    g.setColour (isClose ? Palette::alert().withAlpha (0.35f) : Palette::amberGhost().brighter (0.2f));
                    g.fillRect (r);
                }

                g.setColour (down ? Palette::bg() : (isClose && over ? Palette::alert() : Palette::amber()));

                auto icon = r.reduced (3.0f);
                const float cx = icon.getCentreX();
                const float cy = icon.getCentreY();
                const float s = juce::jmin (icon.getWidth(), icon.getHeight()) * 0.5f;

                if (type == juce::DocumentWindow::minimiseButton)
                {
                    g.fillRect (cx - s, cy + s * 0.5f, s * 2.0f, 1.6f);
                }
                else if (type == juce::DocumentWindow::maximiseButton)
                {
                    g.drawRect (juce::Rectangle<float> (cx - s, cy - s, s * 2.0f, s * 2.0f), 1.4f);
                }
                else
                {
                    g.drawLine (cx - s, cy - s, cx + s, cy + s, 1.6f);
                    g.drawLine (cx - s, cy + s, cx + s, cy - s, 1.6f);
                }
            }

        private:
            int type;
        };
    }

    ConsoleLookAndFeel::ConsoleLookAndFeel()
    {
        applyTheme();
    }

    void ConsoleLookAndFeel::applyTheme()
    {
        setColour (juce::ResizableWindow::backgroundColourId, Palette::bg());
        setColour (juce::Label::textColourId,                 Palette::amber());
        setColour (juce::TextButton::buttonColourId,          Palette::bgPanelAlt());
        setColour (juce::TextButton::buttonOnColourId,        Palette::amberDim());
        setColour (juce::TextButton::textColourOffId,         Palette::amber());
        setColour (juce::TextButton::textColourOnId,          Palette::bg());
        setColour (juce::ComboBox::backgroundColourId,        Palette::bgPanelAlt());
        setColour (juce::ComboBox::textColourId,              Palette::amber());
        setColour (juce::ComboBox::outlineColourId,           Palette::amberDim());
        setColour (juce::ComboBox::arrowColourId,             Palette::amber());
        setColour (juce::ComboBox::buttonColourId,            Palette::bgPanelAlt());
        setColour (juce::PopupMenu::backgroundColourId,       Palette::bgPanel());
        setColour (juce::PopupMenu::textColourId,             Palette::amber());
        setColour (juce::PopupMenu::highlightedBackgroundColourId, Palette::amberDim());
        setColour (juce::PopupMenu::highlightedTextColourId,  Palette::bg());
        setColour (juce::Slider::textBoxTextColourId,         Palette::amber());
        setColour (juce::Slider::textBoxBackgroundColourId,   Palette::bgPanelAlt());
        setColour (juce::Slider::textBoxOutlineColourId,      Palette::amberFaint());
        setColour (juce::Slider::thumbColourId,               Palette::amber());
        setColour (juce::Slider::trackColourId,               Palette::amberDim());
        setColour (juce::Slider::backgroundColourId,          Palette::amberGhost());
        setColour (juce::TextEditor::backgroundColourId,      Palette::bgPanelAlt());
        setColour (juce::TextEditor::textColourId,            Palette::amber());
        setColour (juce::TextEditor::highlightColourId,       Palette::amberDim());
        setColour (juce::TextEditor::outlineColourId,         Palette::amberFaint());
        setColour (juce::ScrollBar::thumbColourId,            Palette::amberDim());
        setColour (juce::ScrollBar::trackColourId,            Palette::bgPanel());
        setColour (juce::TooltipWindow::backgroundColourId,   Palette::bgPanel());
        setColour (juce::TooltipWindow::textColourId,         Palette::amber());
        setColour (juce::TooltipWindow::outlineColourId,      Palette::amberDim());
    }

    juce::Font ConsoleLookAndFeel::getLabelFont (juce::Label&)                  { return monoFont (13.0f); }
    juce::Font ConsoleLookAndFeel::getComboBoxFont (juce::ComboBox&)            { return monoFont (13.0f); }
    juce::Font ConsoleLookAndFeel::getTextButtonFont (juce::TextButton&, int h) { return monoFont (juce::jmin (14.0f, (float) h * 0.55f)); }
    juce::Font ConsoleLookAndFeel::getPopupMenuFont()                          { return monoFont (13.5f); }
    juce::Font ConsoleLookAndFeel::getSliderPopupFont (juce::Slider&)          { return monoFont (11.0f); }

    void ConsoleLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b,
                                                   const juce::Colour&, bool over, bool down)
    {
        auto r = b.getLocalBounds().toFloat().reduced (0.5f);
        auto base = Palette::bgPanelAlt();

        if (down)        base = Palette::amberDim().withAlpha (0.5f);
        else if (over)   base = Palette::amberGhost().brighter (0.15f);

        g.setColour (base);
        g.fillRect (r);

        g.setColour (b.getToggleState() ? Palette::amberBright() : (over ? Palette::amber() : Palette::amberDim()));
        g.drawRect (r, 1.0f);

        if (b.getToggleState())
        {
            g.setColour (Palette::amber().withAlpha (0.12f));
            g.fillRect (r.reduced (1.0f));
        }
    }

    void ConsoleLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
    {
        g.setFont (getTextButtonFont (b, b.getHeight()));
        g.setColour (b.getToggleState() ? Palette::amberBright()
                                        : (b.isEnabled() ? Palette::amber() : Palette::amberDim()));
        g.drawFittedText (b.getButtonText(), b.getLocalBounds(), juce::Justification::centred, 1);
    }

    void ConsoleLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                           int, int, int, int, juce::ComboBox& box)
    {
        auto r = juce::Rectangle<int> (0, 0, width, height);
        g.setColour (Palette::bgPanelAlt());
        g.fillRect (r);
        g.setColour (box.hasKeyboardFocus (false) ? Palette::amber() : Palette::amberDim());
        g.drawRect (r, 1);

        juce::Path arrow;
        auto cx = (float) width - 12.0f;
        auto cy = (float) height * 0.5f;
        arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
        arrow.lineTo (cx, cy + 3.0f);
        arrow.lineTo (cx + 4.0f, cy - 2.0f);
        g.setColour (Palette::amber());
        g.strokePath (arrow, juce::PathStrokeType (1.4f));
    }

    void ConsoleLookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
    {
        label.setBounds (6, 1, box.getWidth() - 24, box.getHeight() - 2);
        label.setFont (getComboBoxFont (box));
        label.setJustificationType (juce::Justification::centredLeft);
    }

    void ConsoleLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float sliderPos, float, float,
                                               juce::Slider::SliderStyle style, juce::Slider&)
    {
        if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearBar)
        {
            auto cy = (float) y + (float) height * 0.5f;
            g.setColour (Palette::amberGhost());
            g.fillRect ((float) x, cy - 2.0f, (float) width, 4.0f);
            g.setColour (Palette::amber().withAlpha (0.85f));
            g.fillRect ((float) x, cy - 2.0f, sliderPos - (float) x, 4.0f);
            g.setColour (Palette::amberBright());
            g.fillRect (sliderPos - 2.0f, (float) y + 2.0f, 4.0f, (float) height - 4.0f);
        }
        else
        {
            auto cx = (float) x + (float) width * 0.5f;
            g.setColour (Palette::amberGhost());
            g.fillRect (cx - 2.0f, (float) y, 4.0f, (float) height);
            g.setColour (Palette::amber().withAlpha (0.85f));
            g.fillRect (cx - 2.0f, sliderPos, 4.0f, (float) (y + height) - sliderPos);
            g.setColour (Palette::amberBright());
            g.fillRect ((float) x + 2.0f, sliderPos - 2.0f, (float) width - 4.0f, 4.0f);
        }
    }

    void ConsoleLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                               float pos, float startAngle, float endAngle, juce::Slider&)
    {
        auto radius = (float) juce::jmin (width, height) * 0.5f - 4.0f;
        auto centreX = (float) x + (float) width * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto angle = startAngle + pos * (endAngle - startAngle);
        auto rf = juce::Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre ({ centreX, centreY });

        g.setColour (Palette::bgPanelAlt());
        g.fillEllipse (rf);
        g.setColour (Palette::amberFaint());
        g.drawEllipse (rf, 1.0f);

        juce::Path arc;
        arc.addCentredArc (centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, startAngle, angle, true);
        g.setColour (Palette::amber());
        g.strokePath (arc, juce::PathStrokeType (2.2f));

        juce::Path needle;
        needle.addRectangle (-1.2f, -radius + 3.0f, 2.4f, radius * 0.45f);
        g.setColour (Palette::amberBright());
        g.fillPath (needle, juce::AffineTransform::rotation (angle).translated (centreX, centreY));
    }

    void ConsoleLookAndFeel::drawScrollbar (juce::Graphics& g, juce::ScrollBar&, int x, int y, int width, int height,
                                            bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                            bool isMouseOver, bool isMouseDown)
    {
        g.setColour (Palette::bgPanel().brighter (0.03f));
        g.fillRect (x, y, width, height);

        juce::Rectangle<int> thumb;
        if (isScrollbarVertical)
            thumb = { x + 2, thumbStartPosition, width - 4, thumbSize };
        else
            thumb = { thumbStartPosition, y + 2, thumbSize, height - 4 };

        g.setColour (isMouseDown ? Palette::amber() : (isMouseOver ? Palette::amberMid() : Palette::amberDim()));
        g.fillRect (thumb);
    }

    void ConsoleLookAndFeel::fillTextEditorBackground (juce::Graphics& g, int width, int height, juce::TextEditor&)
    {
        g.setColour (Palette::bgPanelAlt());
        g.fillRect (0, 0, width, height);
    }

    void ConsoleLookAndFeel::drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor&)
    {
        g.setColour (Palette::amberFaint());
        g.drawRect (0, 0, width, height, 1);
    }

    void ConsoleLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
    {
        g.fillAll (Palette::bgPanel());
        g.setColour (Palette::amberDim());
        g.drawRect (0, 0, width, height, 1);
    }

    void ConsoleLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                                bool isSeparator, bool isActive, bool isHighlighted,
                                                bool isTicked, bool, const juce::String& text,
                                                const juce::String&, const juce::Drawable*, const juce::Colour*)
    {
        if (isSeparator)
        {
            g.setColour (Palette::amberFaint());
            g.fillRect (area.reduced (6, 0).removeFromTop (1));
            return;
        }

        auto r = area.reduced (4, 1);
        if (isHighlighted && isActive)
        {
            g.setColour (Palette::amberDim());
            g.fillRect (r);
        }

        g.setFont (monoFont (13.5f, isTicked));
        g.setColour (isActive ? (isHighlighted ? Palette::bg() : Palette::amber()) : Palette::amberDim());
        g.drawFittedText (text, r.reduced (6, 0), juce::Justification::centredLeft, 1);

        if (isTicked)
        {
            g.setColour (isHighlighted ? Palette::bg() : Palette::amberBright());
            g.drawText (">", r.removeFromRight (12), juce::Justification::centred);
        }
    }

    juce::Button* ConsoleLookAndFeel::createDocumentWindowButton (int buttonType)
    {
        return new WindowGlyphButton (buttonType);
    }

    void ConsoleLookAndFeel::drawDocumentWindowTitleBar (juce::DocumentWindow& window, juce::Graphics& g,
                                                         int w, int h, int titleSpaceX, int titleSpaceW,
                                                         const juce::Image*, bool)
    {
        g.setColour (Palette::bgPanel());
        g.fillRect (0, 0, w, h);

        // amber base line
        g.setColour (Palette::amberDim());
        g.fillRect (0, h - 2, w, 2);
        g.setColour (Palette::amber());
        g.fillRect (6, h - 2, juce::jmax (0, w * 2 / 5), 2);

        // title
        g.setColour (Palette::amberBright());
        g.setFont (monoFont (13.5f, true));
        const int x = juce::jmax (8, titleSpaceX);
        const int tw = titleSpaceW > 0 ? titleSpaceW : w - x - 8;
        g.drawFittedText (window.getName(), x, 2, tw, h - 6, juce::Justification::centredLeft, 1);

        // decorative left tag
        g.setColour (Palette::amberDim());
        g.setFont (monoFont (9.0f));
        g.drawFittedText ("VOX-CONTROL", 8, 2, 90, h - 6, juce::Justification::centredLeft, 1);
    }
}
