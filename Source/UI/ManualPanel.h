#pragma once

#include "Panel.h"

namespace vp
{
    // Touch / mouse operator manual (replaces the keyboard help pane).
    class ManualPanel : public Panel
    {
    public:
        ManualPanel();

    private:
        void paintPanelContent (juce::Graphics&) override;
    };
}
