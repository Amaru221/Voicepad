#include "Toolbar.h"

namespace vp
{
    Toolbar::Toolbar (AudioEngine& engineToUse, VoiceParams& paramsToUse)
        : engine (engineToUse), params (paramsToUse)
    {
        epochStart = juce::Time::getCurrentTime().toMilliseconds() / 1000.0;

        powerButton.setColour (juce::TextButton::buttonColourId, Palette::bgPanel());
        powerButton.onClick = [this]
        {
            params.bypass.store (! params.bypass.load());
            if (onPowerToggled != nullptr)
                onPowerToggled();
        };
        addAndMakeVisible (powerButton);

        themeLabel.setText ("SKIN", juce::dontSendNotification);
        themeLabel.setFont (monoFont (10.0f, true));
        themeLabel.setColour (juce::Label::textColourId, Palette::amberDim());
        themeLabel.setJustificationType (juce::Justification::centredRight);
        addAndMakeVisible (themeLabel);

        const auto themeNames = ThemeManager::names();
        for (int i = 0; i < themeNames.size(); ++i)
            themeBox.addItem (themeNames[i], i + 1);
        themeBox.setSelectedItemIndex (ThemeManager::getCurrentIndex(), juce::dontSendNotification);
        themeBox.onChange = [this]
        {
            const int idx = themeBox.getSelectedItemIndex();
            if (idx >= 0 && onThemeChanged != nullptr)
                onThemeChanged (idx);
        };
        addAndMakeVisible (themeBox);

        startTimerHz (10);
    }

    void Toolbar::setThemeIndex (int index)
    {
        themeBox.setSelectedItemIndex (index, juce::dontSendNotification);
    }

    void Toolbar::resized()
    {
        auto r = getLocalBounds().reduced (6, 3);
        powerButton.setBounds (r.removeFromRight (150));
        r.removeFromRight (10);
        themeBox.setBounds (r.removeFromRight (150));
        themeLabel.setBounds (r.removeFromRight (42));
    }

    void Toolbar::timerCallback()
    {
        const bool bypass = params.bypass.load();
        const juce::String text = bypass ? "PROCESS: BYPASS" : "PROCESS: ON";
        if (powerButton.getButtonText() != text)
            powerButton.setButtonText (text);

        repaint();
    }

    void Toolbar::drawLamp (juce::Graphics& g, juce::Rectangle<int> area, const juce::String& label, bool on, juce::Colour colour)
    {
        auto dot = area.removeFromLeft (area.getHeight()).reduced (3);
        g.setColour (on ? colour : Palette::amberFaint());
        g.fillEllipse (dot.toFloat());

        g.setColour (Palette::amberDim());
        g.setFont (monoFont (9.5f));
        g.drawFittedText (label, area.withTrimmedLeft (2).removeFromRight (64), juce::Justification::centredLeft, 1);
    }

    void Toolbar::paint (juce::Graphics& g)
    {
        auto bounds = getLocalBounds();
        g.setColour (Palette::bgPanel());
        g.fillRect (bounds);
        g.setColour (Palette::amberFaint());
        g.fillRect (bounds.getX(), bounds.getBottom() - 1, bounds.getWidth(), 1);

        auto left = bounds.reduced (8, 0).removeFromLeft (400);
        g.setColour (Palette::amberBright());
        g.setFont (monoFont (15.0f, true));
        g.drawFittedText ("VOICEPAD // VOX-CONTROL CONSOLE", left, juce::Justification::centredLeft, 1);

        auto right = bounds.reduced (8, 0);
        auto statusArea = right.withTrimmedRight (170).removeFromRight (300);
        int x = statusArea.getX();
        const int lampW = 96;
        drawLamp (g, { x, statusArea.getY(), lampW, statusArea.getHeight() }, "ENGINE", engine.isRunning(), Palette::ok());
        drawLamp (g, { x + lampW, statusArea.getY(), lampW, statusArea.getHeight() }, "VOICE", engine.getVoiceActive(), Palette::amber());
        drawLamp (g, { x + lampW * 2, statusArea.getY(), lampW, statusArea.getHeight() }, "LINK", engine.isRunning() && engine.getOutputLevel() > 0.001f, Palette::amberBright());

        auto epochArea = bounds.reduced (8, 0);
        epochArea = epochArea.withTrimmedRight (160).removeFromRight (210);
        const double now = juce::Time::getCurrentTime().toMilliseconds() / 1000.0;
        const auto clockString = juce::Time::getCurrentTime().toString (false, true, true, false);
        g.setColour (Palette::amberMid());
        g.setFont (monoFont (11.0f));
        g.drawFittedText ("EPOCH " + juce::String ((juce::int64) now), epochArea.removeFromTop (epochArea.getHeight() / 2),
                          juce::Justification::centredRight, 1);
        g.setColour (Palette::amberDim());
        g.drawFittedText ("LOCAL " + clockString, epochArea,
                          juce::Justification::centredRight, 1);
    }
}
