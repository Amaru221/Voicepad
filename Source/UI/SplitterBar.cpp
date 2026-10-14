#include "SplitterBar.h"

namespace vp
{
    SplitterBar::SplitterBar (Orientation o) : orientation (o)
    {
        setMouseCursor (o == Orientation::Vertical ? juce::MouseCursor::LeftRightResizeCursor
                                                   : juce::MouseCursor::UpDownResizeCursor);
    }

    void SplitterBar::paint (juce::Graphics& g)
    {
        g.fillAll (Palette::bg());

        g.setColour (hover || dragging ? Palette::amberDim() : Palette::amberGhost());

        if (orientation == Orientation::Vertical)
        {
            const float cx = (float) getWidth() * 0.5f;
            g.fillRect (cx - 0.5f, (float) getY() + 4.0f, 1.0f, (float) getHeight() - 8.0f);

            const int cy = getHeight() / 2;
            g.setColour (hover || dragging ? Palette::amber() : Palette::amberDim());
            for (int i = -2; i <= 2; ++i)
                g.fillRect (cx - 2.0f, (float) (cy + i * 3 - 1), 4.0f, 1.0f);
        }
        else
        {
            const float cy = (float) getHeight() * 0.5f;
            g.fillRect ((float) getX() + 4.0f, cy - 0.5f, (float) getWidth() - 8.0f, 1.0f);

            const int cx = getWidth() / 2;
            g.setColour (hover || dragging ? Palette::amber() : Palette::amberDim());
            for (int i = -2; i <= 2; ++i)
                g.fillRect ((float) (cx + i * 3 - 1), cy - 2.0f, 1.0f, 4.0f);
        }
    }

    void SplitterBar::mouseDown (const juce::MouseEvent& e)
    {
        dragging = true;
        lastScreenPos = e.getScreenPosition();
        repaint();
    }

    void SplitterBar::mouseDrag (const juce::MouseEvent& e)
    {
        const auto cur = e.getScreenPosition();
        const int delta = (orientation == Orientation::Vertical) ? (cur.x - lastScreenPos.x)
                                                                 : (cur.y - lastScreenPos.y);
        lastScreenPos = cur;

        if (delta != 0 && onDrag != nullptr)
            onDrag (delta);
    }

    void SplitterBar::mouseEnter (const juce::MouseEvent&) { hover = true; repaint(); }

    void SplitterBar::mouseUp (const juce::MouseEvent&) { dragging = false; repaint(); }

    void SplitterBar::mouseExit (const juce::MouseEvent&) { hover = false; dragging = false; repaint(); }
}
