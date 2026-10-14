#include "GlobalHotkeys.h"

#include <atomic>
#include <chrono>
#include <thread>

#if JUCE_WINDOWS
 #include <windows.h>
#endif

namespace vp
{
    struct GlobalHotkeys::Impl
    {
        std::thread thread;
        std::atomic<bool> quit { false };
#if JUCE_WINDOWS
        std::atomic<unsigned long> threadId { 0 };
#endif
    };

    GlobalHotkeys::GlobalHotkeys()
        : impl (std::make_unique<Impl>())
    {
        bindings = defaultBindings (8);
        start();
    }

    GlobalHotkeys::~GlobalHotkeys()
    {
        stop();
    }

    std::vector<GlobalHotkeys::Binding> GlobalHotkeys::defaultBindings (int numPresets)
    {
        std::vector<Binding> b;
#if JUCE_WINDOWS
        const unsigned int mods = MOD_CONTROL | MOD_SHIFT | MOD_NOREPEAT;

        for (int i = 0; i < numPresets; ++i)
        {
            Binding bind;
            bind.modifiers = mods;
            bind.vk = (unsigned int) ('1' + i);
            bind.action = i;
            bind.label = "PRESET " + juce::String (i + 1);
            b.push_back (bind);
        }

        b.push_back ({ mods, (unsigned int) '0', BypassAction,  "BYPASS TOGGLE" });
        b.push_back ({ mods, (unsigned int) 'S', StopAllAction, "STOP SOUNDBOARD" });
#else
        juce::ignoreUnused (numPresets);
#endif
        return b;
    }

    void GlobalHotkeys::setActionCallback (std::function<void (int)> cb)
    {
        juce::SpinLock::ScopedLockType sl (callbackLock);
        actionCallback = std::move (cb);
    }

    void GlobalHotkeys::start()
    {
        impl->quit.store (false);
        impl->thread = std::thread ([this]
        {
#if JUCE_WINDOWS
            impl->threadId.store ((unsigned long) GetCurrentThreadId());

            int id = 1;
            std::vector<std::pair<int, int>> idToAction;
            for (const auto& bind : bindings)
            {
                if (RegisterHotKey (nullptr, id, bind.modifiers, bind.vk))
                    idToAction.emplace_back (id, bind.action);
                ++id;
            }

            MSG msg;
            while (! impl->quit.load() && GetMessage (&msg, nullptr, 0, 0) > 0)
            {
                if (msg.message == WM_HOTKEY)
                {
                    const int wid = (int) msg.wParam;
                    for (const auto& pr : idToAction)
                        if (pr.first == wid)
                            fire (pr.second);
                }
            }

            for (const auto& pr : idToAction)
                UnregisterHotKey (nullptr, pr.first);
#else
            while (! impl->quit.load())
                std::this_thread::sleep_for (std::chrono::milliseconds (100));
#endif
        });
    }

    void GlobalHotkeys::stop()
    {
        if (impl == nullptr || ! impl->thread.joinable())
            return;

        impl->quit.store (true);
#if JUCE_WINDOWS
        if (impl->threadId.load() != 0)
            PostThreadMessage ((DWORD) impl->threadId.load(), WM_QUIT, 0, 0);
#endif
        impl->thread.join();
    }

    void GlobalHotkeys::fire (int action)
    {
        std::function<void (int)> cb;
        {
            juce::SpinLock::ScopedLockType sl (callbackLock);
            cb = actionCallback;
        }

        if (cb != nullptr)
            juce::MessageManager::callAsync ([cb, action] { cb (action); });
    }
}
