#pragma once

#include "Common.h"

namespace vp
{
    // System-wide hotkeys (Win32 RegisterHotKey) delivered to the message thread.
    class GlobalHotkeys
    {
    public:
        struct Binding
        {
            unsigned int modifiers = 0;
            unsigned int vk = 0;
            int action = 0;
            juce::String label;
        };

        enum ActionId { BypassAction = 100, StopAllAction = 101 };

        GlobalHotkeys();
        ~GlobalHotkeys();

        void setActionCallback (std::function<void (int)> cb);

        const std::vector<Binding>& getBindings() const noexcept { return bindings; }
        static std::vector<Binding> defaultBindings (int numPresets);

    private:
        void start();
        void stop();
        void fire (int action);

        struct Impl;
        std::unique_ptr<Impl> impl;
        std::vector<Binding> bindings;
        std::function<void (int)> actionCallback;
        juce::SpinLock callbackLock;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalHotkeys)
    };
}
