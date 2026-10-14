#pragma once

#include "Panel.h"
#include "Audio/AudioEngine.h"

namespace vp
{
    // Central signal map: live waveform scope with interactive numeric markers.
    // Interaction: drag to pan the view window, double-click to cycle zoom (mouse/touch only).
    class ScopePanel : public Panel, private juce::Timer
    {
    public:
        explicit ScopePanel (AudioEngine& engineToUse);

        void paintPanelContent (juce::Graphics&) override;

        void resetView();

        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;
        void mouseUp (const juce::MouseEvent&) override;
        void mouseDoubleClick (const juce::MouseEvent&) override;
        void mouseMove (const juce::MouseEvent&) override;
        void mouseExit (const juce::MouseEvent&) override;

    private:
        void timerCallback() override;
        juce::Rectangle<int> getScopeArea() const;
        float markerX (int index) const;
        int markerAt (juce::Point<int>) const;

        AudioEngine& engine;
        ScopeData& scopeData;
        std::vector<float> data;

        static constexpr int numMarkers = 6;
        float markerPos[numMarkers] = { 0.16f, 0.31f, 0.45f, 0.58f, 0.71f, 0.85f };

        float pan = 0.0f;
        float zoom = 1.0f;
        bool dragging = false;
        float dragStartPan = 0.0f;
        int dragStartX = 0;
        int hoverMarker = -1;
        int selectedMarker = -1;
    };
}
