#include "EffectsPanel.h"

namespace vp
{
    EffectsPanel::EffectsPanel (VoiceParams& paramsToUse)
        : Panel ("EFFECT MATRIX"), params (paramsToUse)
    {
        addControl ("PITCH",   &params.pitchSemis,      -12.0f, 12.0f, " st", 2);
        addControl ("FORMANT", &params.formantSemis,     -6.0f,  6.0f, " st", 2);
        addControl ("GATE",    &params.gateThreshold,   -80.0f, -10.0f, " dB", 1);
        addControl ("DRIVE",   &params.drive,             0.0f,  1.0f, "",    2);
        addControl ("ROBOT",   &params.robotMix,          0.0f,  1.0f, "",    2);
        addControl ("CHORUS",  &params.chorusMix,         0.0f,  1.0f, "",    2);
        addControl ("ECHO",    &params.echoMix,           0.0f,  1.0f, "",    2);
        addControl ("REVERB",  &params.reverbMix,         0.0f,  1.0f, "",    2);
        addControl ("COMP",    &params.compAmount,        0.0f,  1.0f, "",    2);
        addControl ("OUT",     &params.outputGainDb,    -24.0f, 12.0f, " dB", 1);
        addControl ("DEN.",    &params.noiseThresholdDb, -70.0f, -10.0f, " dB", 1);
        addControl ("BOARD",   &params.soundboardGainDb, -24.0f, 12.0f, " dB", 1);

        noiseButton.onClick = [this]
        {
            params.noiseCancel.store (! params.noiseCancel.load());
            updateNoiseButton();
        };
        noiseButton.setColour (juce::TextButton::buttonOnColourId, Palette::amberDim());
        addAndMakeVisible (noiseButton);
        updateNoiseButton();

        startTimerHz (12);
    }

    void EffectsPanel::updateNoiseButton()
    {
        const bool on = params.noiseCancel.load();
        noiseButton.setToggleState (on, juce::dontSendNotification);
        noiseButton.setButtonText (on ? "NOISE CANCEL: ON" : "NOISE CANCEL: OFF");
    }

    void EffectsPanel::addControl (juce::String name, std::atomic<float>* target,
                                   float min, float max, juce::String suffix, int decimalsToUse)
    {
        auto* lbl = labels.add (new juce::Label());
        lbl->setText (name, juce::dontSendNotification);
        lbl->setFont (monoFont (10.0f, true));
        lbl->setColour (juce::Label::textColourId, Palette::amberMid());
        lbl->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (lbl);

        auto* s = sliders.add (new juce::Slider());
        s->setSliderStyle (juce::Slider::LinearHorizontal);
        s->setTextBoxStyle (juce::Slider::TextBoxRight, false, 56, 17);
        s->setRange ((double) min, (double) max, 0.01);
        s->setNumDecimalPlacesToDisplay (decimalsToUse);
        s->setTextValueSuffix (suffix);
        s->setValue ((double) target->load(), juce::dontSendNotification);
        s->setColour (juce::Slider::textBoxTextColourId, Palette::amberBright());
        addAndMakeVisible (s);

        const int index = (int) targets.size();
        targets.push_back (target);
        decimals.push_back (decimalsToUse);

        s->onValueChange = [this, index, s]
        {
            targets[(size_t) index]->store ((float) s->getValue());
        };

        s->setDoubleClickReturnValue (true, (double) ((min + max) * 0.5f));
    }

    void EffectsPanel::resized()
    {
        auto r = getContentBounds();

        auto top = r.removeFromTop (24);
        noiseButton.setBounds (top.removeFromLeft (juce::jmin (180, top.getWidth())));
        r.removeFromTop (4);

        const int rows = 6;
        const int cols = 2;
        const int rowH = r.getHeight() / rows;
        const int colW = r.getWidth() / cols;

        for (int i = 0; i < sliders.size(); ++i)
        {
            const int col = i / rows;
            const int row = i % rows;
            auto cell = juce::Rectangle<int> (r.getX() + col * colW, r.getY() + row * rowH, colW, rowH).reduced (4, 3);
            labels[i]->setBounds (cell.removeFromLeft (56));
            sliders[i]->setBounds (cell);
        }
    }

    void EffectsPanel::timerCallback()
    {
        for (int i = 0; i < sliders.size(); ++i)
        {
            const float v = targets[(size_t) i]->load();
            if (std::abs ((float) sliders[i]->getValue() - v) > 0.0001f)
                sliders[i]->setValue ((double) v, juce::dontSendNotification);
        }

        updateNoiseButton();
    }
}
