#pragma once

#include "Common.h"

namespace vp
{
    // Thin draggable divider between mosaic panels (mouse / touch only).
    class SplitterBar : public juce::Component
    {
    public:
        enum class Orientation { Vertical, Horizontal };

        explicit SplitterBar (Orientation o);

        // delta is the pointer movement in pixels since the previous drag event.
        std::function<void (int delta)> onDrag;

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseEnter (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;

    private:
        Orientation orientation;
        juce::Point<int> lastScreenPos;
        bool hover = false;
        bool dragging = false;
    };
}
