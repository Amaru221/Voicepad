#include "Common.h"
#include "MainComponent.h"
#include "Tests/SelfTest.h"

class VoicePadApplication : public juce::JUCEApplication
{
public:
    VoicePadApplication() = default;

    const juce::String getApplicationName() override    { return "VoicePad"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override          { return true; }

    void initialise (const juce::String& commandLine) override
    {
        if (commandLine.contains ("--selftest"))
        {
            const auto outFile = juce::File::getSpecialLocation (juce::File::tempDirectory)
                                     .getChildFile ("voicepad_selftest.txt");
            setApplicationReturnValue (vp::runSelfTests (outFile));
            quit();
            return;
        }

        mainWindow = std::make_unique<MainWindow> (getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name)
            : DocumentWindow (name, vp::Palette::bg(), DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (false);
            setTitleBarHeight (30);
            setContentOwned (new vp::MainComponent(), true);
            setResizable (true, false);
            setResizeLimits (1180, 760, 10000, 10000);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (VoicePadApplication)
