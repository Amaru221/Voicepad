#include "DevicePanel.h"

namespace vp
{
    DevicePanel::DevicePanel (AudioEngine& engineToUse)
        : Panel ("DEVICE ROUTING"), engine (engineToUse)
    {
        inLabel.setText ("INPUT  // MIC SOURCE", juce::dontSendNotification);
        outLabel.setText ("OUTPUT // VIRTUAL CABLE", juce::dontSendNotification);
        inLabel.setFont (monoFont (10.0f, true));
        outLabel.setFont (monoFont (10.0f, true));
        inLabel.setColour (juce::Label::textColourId, Palette::amberDim());
        outLabel.setColour (juce::Label::textColourId, Palette::amberDim());

        addAndMakeVisible (inLabel);
        addAndMakeVisible (outLabel);
        addAndMakeVisible (inputBox);
        addAndMakeVisible (outputBox);
        addAndMakeVisible (autoButton);
        addAndMakeVisible (refreshButton);
        addAndMakeVisible (startButton);

        autoButton.onClick = [this]
        {
            if (engine.autoRoute())
            {
                refreshLists();
                if (onLog != nullptr) onLog ("AUTOROUTE COMPLETE");
            }
            else if (onLog != nullptr)
            {
                onLog ("AUTOROUTE FAILED: VB-CABLE NOT DETECTED");
            }
        };

        refreshButton.onClick = [this] { refreshLists(); };

        startButton.onClick = [this]
        {
            if (engine.isRunning())
            {
                engine.stopAudio();
                if (onLog != nullptr) onLog ("ENGINE DISENGAGED");
            }
            else
            {
                applySelection();
                engine.startAudio();
                if (onLog != nullptr) onLog ("ENGINE ENGAGED");
            }
            updateStartButton();
        };

        inputBox.onChange = [this] { applySelection(); };
        outputBox.onChange = [this] { applySelection(); };

        refreshLists();
        updateStartButton();
    }

    void DevicePanel::refreshLists()
    {
        inputBox.clear (juce::dontSendNotification);
        outputBox.clear (juce::dontSendNotification);

        const auto inputs = engine.getInputDeviceNames();
        for (int i = 0; i < inputs.size(); ++i)
            inputBox.addItem (inputs[i], i + 1);

        const auto outputs = engine.getOutputDeviceNames();
        for (int i = 0; i < outputs.size(); ++i)
            outputBox.addItem (outputs[i], i + 1);

        const auto inName = engine.getInputDeviceName();
        const auto outName = engine.getOutputDeviceName();

        if (inName.isNotEmpty())
            inputBox.setText (inName, juce::dontSendNotification);
        if (outName.isNotEmpty())
            outputBox.setText (outName, juce::dontSendNotification);
    }

    void DevicePanel::applySelection()
    {
        const auto inName = inputBox.getText();
        const auto outName = outputBox.getText();
        if (inName.isNotEmpty() && outName.isNotEmpty())
            engine.applySetup (inName, outName);
    }

    void DevicePanel::updateStartButton()
    {
        startButton.setButtonText (engine.isRunning() ? "DISENGAGE" : "ENGAGE");
        startButton.setToggleState (engine.isRunning(), juce::dontSendNotification);
    }

    void DevicePanel::resized()
    {
        auto r = getContentBounds();
        const int rowH = 26;

        inLabel.setBounds (r.removeFromTop (14));
        inputBox.setBounds (r.removeFromTop (rowH));
        r.removeFromTop (6);
        outLabel.setBounds (r.removeFromTop (14));
        outputBox.setBounds (r.removeFromTop (rowH));
        r.removeFromTop (8);

        auto btnRow = r.removeFromTop (24);
        const int bw = (btnRow.getWidth() - 12) / 3;
        autoButton.setBounds (btnRow.removeFromLeft (bw));
        btnRow.removeFromLeft (6);
        refreshButton.setBounds (btnRow.removeFromLeft (bw));
        btnRow.removeFromLeft (6);
        startButton.setBounds (btnRow);
    }

    void DevicePanel::paintPanelContent (juce::Graphics& g)
    {
        auto r = getContentBounds();
        auto footer = r.removeFromBottom (16);

        g.setFont (monoFont (9.5f));
        g.setColour (engine.isRunning() ? Palette::ok() : Palette::alert());
        g.drawFittedText (engine.isRunning() ? "STREAM ACTIVE" : "STREAM HALTED",
                          footer.removeFromLeft (120), juce::Justification::centredLeft, 1);

        g.setColour (Palette::amberDim());
        g.drawFittedText (juce::String (engine.getCurrentSampleRate(), 0) + " HZ / "
                              + juce::String (engine.getCurrentBlockSize()) + " SMP",
                          footer, juce::Justification::centredRight, 1);

        if (! engine.isRunning())
        {
            g.setColour (Palette::amberFaint());
            g.setFont (monoFont (9.0f));
            g.drawFittedText ("SELECT  CABLE INPUT  AS ROUTE OUTPUT",
                              r.removeFromBottom (14), juce::Justification::centredLeft, 1);
        }
    }
}
