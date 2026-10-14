#include "PresetPanel.h"

namespace vp
{
    PresetPanel::PresetPanel (PresetManager& presetsToUse, VoiceParams& paramsToUse, EventLog& logToUse)
        : Panel ("VOICE PRESETS"), presets (presetsToUse), params (paramsToUse), log (logToUse)
    {
        for (int i = 0; i < presets.getNumPresets(); ++i)
        {
            auto* b = pads.add (new juce::TextButton (presets.getDescriptor (i).name));
            b->setClickingTogglesState (false);
            b->onClick = [this, i] { selectPreset (i, true); };
            addAndMakeVisible (b);
        }

        selected = presets.getSelected();
        startTimerHz (6);
    }

    void PresetPanel::selectPreset (int index, bool notify)
    {
        if (! juce::isPositiveAndBelow (index, pads.size()))
            return;

        selected = index;
        presets.setSelected (index);
        presets.applyPreset (index, params);

        if (notify)
            log.add ("PRESET", "VOICE", presets.getDescriptor (index).name, 1);

        for (int i = 0; i < pads.size(); ++i)
            pads[i]->setToggleState (i == index, juce::dontSendNotification);

        if (onPresetApplied != nullptr)
            onPresetApplied (index);

        repaint();
    }

    void PresetPanel::timerCallback()
    {
        repaint();
    }

    void PresetPanel::highlightOnly (int index)
    {
        if (! juce::isPositiveAndBelow (index, pads.size()))
            return;

        selected = index;
        presets.setSelected (index);

        for (int i = 0; i < pads.size(); ++i)
            pads[i]->setToggleState (i == index, juce::dontSendNotification);

        repaint();
    }

    void PresetPanel::resized()
    {
        auto r = getContentBounds();
        auto grid = r.removeFromTop (74);

        const int cols = 4;
        const int rows = 2;
        const int padW = grid.getWidth() / cols;
        const int padH = grid.getHeight() / rows;

        for (int i = 0; i < pads.size(); ++i)
        {
            const int c = i % cols;
            const int rr = i / cols;
            pads[i]->setBounds (grid.getX() + c * padW + 3,
                                grid.getY() + rr * padH + 3,
                                padW - 6, padH - 6);
        }
    }

    void PresetPanel::paintPanelContent (juce::Graphics& g)
    {
        auto r = getContentBounds();
        r.removeFromTop (74 + 8);

        g.setColour (Palette::amberDim());
        g.drawLine ((float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getY());
        r.removeFromTop (6);

        const auto desc = presets.getDescriptor (selected);
        const auto& p = presets.getPreset (selected);

        auto header = r.removeFromTop (16);
        g.setColour (Palette::amberBright());
        g.setFont (monoFont (12.0f, true));
        g.drawFittedText (desc.name, header.removeFromLeft (120), juce::Justification::centredLeft, 1);
        g.setColour (Palette::amberDim());
        g.setFont (monoFont (10.0f));
        g.drawFittedText ("[" + desc.tag + "]", header, juce::Justification::centredLeft, 1);

        auto descArea = r.removeFromTop (36);
        g.setColour (Palette::amberMid());
        g.setFont (monoFont (10.0f));

        juce::AttributedString as;
        as.setJustification (juce::Justification::topLeft);
        as.append (desc.description, monoFont (10.0f), Palette::amberMid());
        juce::TextLayout tl;
        tl.createLayout (as, (float) descArea.getWidth());
        tl.draw (g, descArea.toFloat());

        r.removeFromTop (4);

        // weights table
        g.setColour (Palette::amberDim());
        g.setFont (monoFont (9.0f, true));
        auto wh = r.removeFromTop (12);
        g.drawFittedText ("FACTOR", wh.removeFromLeft (90), juce::Justification::centredLeft, 1);
        g.drawFittedText ("WEIGHT", wh, juce::Justification::centredRight, 1);

        struct Row { const char* name; float value; float min; float max; };
        const Row rowsv[] = {
            { "PITCH",   p.pitchSemis,   -12.0f, 12.0f },
            { "FORMANT", p.formantSemis, -6.0f,  6.0f  },
            { "DRIVE",   p.drive,         0.0f,  1.0f  },
            { "ROBOT",   p.robotMix,      0.0f,  1.0f  },
            { "REVERB",  p.reverbMix,     0.0f,  1.0f  },
            { "ECHO",    p.echoMix,       0.0f,  1.0f  },
        };

        for (const auto& row : rowsv)
        {
            auto line = r.removeFromTop (13);
            g.setColour (Palette::amberMid());
            g.setFont (monoFont (9.5f));
            g.drawFittedText (row.name, line.removeFromLeft (90), juce::Justification::centredLeft, 1);

            auto barArea = line.withTrimmedRight (34);
            const float t = juce::jlimit (0.0f, 1.0f, (row.value - row.min) / (row.max - row.min));
            g.setColour (Palette::amberGhost());
            g.fillRect (barArea.reduced (0, 4));
            g.setColour (Palette::amber());
            g.fillRect (barArea.reduced (0, 4).withWidth ((int) (barArea.getWidth() * t)));

            g.setColour (Palette::amberBright());
            g.setFont (monoFont (9.0f));
            g.drawFittedText (juce::String (row.value, 2), line, juce::Justification::centredRight, 1);
        }

        // timeline
        r.removeFromTop (4);
        auto tlTitle = r.removeFromTop (12);
        g.setColour (Palette::amberDim());
        g.setFont (monoFont (9.0f, true));
        g.drawFittedText ("EVENT TIMELINE", tlTitle, juce::Justification::centredLeft, 1);

        const auto& entries = log.getEntries();
        const int shown = juce::jmin (4, (int) entries.size());
        for (int i = 0; i < shown; ++i)
        {
            const auto& e = entries[entries.size() - 1 - (size_t) i];
            auto line = r.removeFromTop (12);
            const auto time = juce::Time ((juce::int64) (e.time * 1000.0));
            g.setColour (Palette::amberDim());
            g.setFont (monoFont (9.0f));
            g.drawFittedText (time.toString (false, true, true, false), line.removeFromLeft (58), juce::Justification::centredLeft, 1);
            g.setColour (e.level == 2 ? Palette::alert() : Palette::amberMid());
            g.drawFittedText (e.message, line, juce::Justification::centredLeft, 1);
        }
    }
}
