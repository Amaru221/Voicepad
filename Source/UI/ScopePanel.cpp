#include "ScopePanel.h"

namespace vp
{
    ScopePanel::ScopePanel (AudioEngine& engineToUse)
        : Panel ("SIGNAL MAP // OSCILLOSCOPE"), engine (engineToUse), scopeData (engineToUse.getScope())
    {
        data.resize ((size_t) ScopeData::size, 0.0f);
        startTimerHz (30);
    }

    void ScopePanel::timerCallback()
    {
        scopeData.snapshot (data);
        repaint();
    }

    void ScopePanel::resetView()
    {
        pan = 0.0f;
        zoom = 1.0f;
        selectedMarker = -1;
        hoverMarker = -1;
        repaint();
    }

    juce::Rectangle<int> ScopePanel::getScopeArea() const
    {
        return getContentBounds();
    }

    float ScopePanel::markerX (int index) const
    {
        auto area = getScopeArea().toFloat();
        const float viewLen = 1.0f / zoom;
        const float viewStart = pan * (1.0f - viewLen);
        const float frac = juce::jlimit (0.0f, 1.0f, (markerPos[index] - viewStart) / viewLen);
        return area.getX() + frac * area.getWidth();
    }

    int ScopePanel::markerAt (juce::Point<int> p) const
    {
        for (int i = 0; i < numMarkers; ++i)
            if (std::abs ((float) p.x - markerX (i)) < 9.0f)
                return i;
        return -1;
    }

    void ScopePanel::paintPanelContent (juce::Graphics& g)
    {
        auto area = getScopeArea();
        if (area.isEmpty())
            return;

        g.saveState();
        g.reduceClipRegion (area);

        // screen background
        g.setColour (Palette::bgScreen());
        g.fillRect (area);

        // grid
        g.setColour (Palette::amberGhost());
        for (int i = 1; i < 8; ++i)
        {
            const float x = (float) area.getX() + (float) area.getWidth() * (float) i / 8.0f;
            g.drawLine (x, (float) area.getY(), x, (float) area.getBottom(), 0.5f);
        }
        for (int i = 1; i < 4; ++i)
        {
            const float y = (float) area.getY() + (float) area.getHeight() * (float) i / 4.0f;
            g.drawLine ((float) area.getX(), y, (float) area.getRight(), y, 0.5f);
        }

        const float midY = (float) area.getCentreY();
        g.setColour (Palette::amberFaint());
        g.drawLine ((float) area.getX(), midY, (float) area.getRight(), midY, 1.0f);

        // waveform
        const int viewLen = juce::jmax (2, (int) ((float) data.size() / zoom));
        const int startIndex = (int) (pan * (float) (data.size() - viewLen));

        g.setColour (Palette::amber());
        const int w = area.getWidth();
        for (int px = 0; px < w; ++px)
        {
            const int s0 = startIndex + (int) ((float) px / (float) w * (float) viewLen);
            const int s1 = startIndex + (int) ((float) (px + 1) / (float) w * (float) viewLen);
            float lo = 1.0f, hi = -1.0f;
            for (int s = s0; s < juce::jmax (s0 + 1, s1) && s < (int) data.size(); ++s)
            {
                lo = juce::jmin (lo, data[(size_t) s]);
                hi = juce::jmax (hi, data[(size_t) s]);
            }
            const float y0 = midY - hi * (float) area.getHeight() * 0.46f;
            const float y1 = midY - lo * (float) area.getHeight() * 0.46f;
            g.drawLine ((float) (area.getX() + px), y0, (float) (area.getX() + px), juce::jmax (y0 + 0.6f, y1), 1.0f);
        }

        // markers
        for (int i = 0; i < numMarkers; ++i)
        {
            const float x = markerX (i);
            if (x < (float) area.getX() - 4.0f || x > (float) area.getRight() + 4.0f)
                continue;

            const bool hot = (i == hoverMarker || i == selectedMarker);
            g.setColour (hot ? Palette::amberBright() : Palette::amber());

            juce::Rectangle<float> dot (x - 6.0f, (float) area.getY() + 6.0f, 12.0f, 12.0f);
            g.drawEllipse (dot, 1.2f);
            if (hot)
                g.fillEllipse (dot.reduced (2.5f));

            g.setFont (monoFont (8.5f, hot));
            g.drawText (juce::String (i + 1), dot.toNearestInt(), juce::Justification::centred);
        }

        // readout
        const int active = (hoverMarker >= 0) ? hoverMarker : selectedMarker;
        if (active >= 0)
        {
            const int s = juce::jlimit (0, (int) data.size() - 1, (int) (markerPos[active] * (float) data.size()));
            const float lvl = std::abs (data[(size_t) s]);
            const float ms = markerPos[active] * 85.0f;
            juce::Rectangle<int> box (area.getX() + 6, area.getBottom() - 30, 190, 24);
            g.setColour (Palette::bgPanel().withAlpha (0.92f));
            g.fillRect (box);
            g.setColour (Palette::amberDim());
            g.drawRect (box, 1);
            g.setColour (Palette::amberBright());
            g.setFont (monoFont (9.5f));
            g.drawFittedText ("MRK-" + juce::String (active + 1) + "  LVL " + juce::String (lvl, 3)
                                  + "  T+" + juce::String (ms, 1) + "ms",
                              box.reduced (6, 0), juce::Justification::centredLeft, 1);
        }

        g.setColour (Palette::amberDim());
        g.setFont (monoFont (9.0f));
        g.drawFittedText ("DRAG=PAN  DBLCLK=ZOOM x" + juce::String ((int) zoom),
                          area.removeFromTop (12).removeFromRight (220), juce::Justification::centredRight, 1);

        g.restoreState();
    }

    void ScopePanel::mouseDown (const juce::MouseEvent& e)
    {
        Panel::mouseDown (e);

        if (! getContentBounds().contains (e.getPosition()))
            return;

        const int m = markerAt (e.getPosition());
        selectedMarker = m;

        if (m < 0)
        {
            dragging = true;
            dragStartPan = pan;
            dragStartX = e.x;
        }
        repaint();
    }

    void ScopePanel::mouseDrag (const juce::MouseEvent& e)
    {
        if (! dragging)
            return;

        auto area = getScopeArea();
        if (area.getWidth() <= 0)
            return;

        const float viewLen = 1.0f / zoom;
        const float delta = (float) (dragStartX - e.x) / (float) area.getWidth() * viewLen;
        pan = juce::jlimit (0.0f, 1.0f - viewLen, dragStartPan + delta / juce::jmax (0.0001f, (1.0f - viewLen)));
        repaint();
    }

    void ScopePanel::mouseUp (const juce::MouseEvent&)
    {
        dragging = false;
    }

    void ScopePanel::mouseDoubleClick (const juce::MouseEvent&)
    {
        zoom = (zoom < 1.5f) ? 4.0f : 1.0f;
        if (zoom < 1.5f)
            pan = 0.0f;
        repaint();
    }

    void ScopePanel::mouseMove (const juce::MouseEvent& e)
    {
        Panel::mouseMove (e);

        const int m = markerAt (e.getPosition());
        if (m != hoverMarker)
        {
            hoverMarker = m;
            repaint();
        }
    }

    void ScopePanel::mouseExit (const juce::MouseEvent& e)
    {
        Panel::mouseExit (e);

        if (hoverMarker != -1)
        {
            hoverMarker = -1;
            repaint();
        }
    }
}
