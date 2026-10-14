#pragma once

#include <vector>
#include <juce_core/juce_core.h>

namespace vp
{
    struct LogEntry
    {
        double time = 0.0;
        juce::String type;
        juce::String source;
        juce::String message;
        int level = 0; // 0 = info, 1 = active, 2 = alert
    };

    // Simple append-only event log (message thread only).
    class EventLog
    {
    public:
        void add (juce::String type, juce::String source, juce::String message, int level = 0)
        {
            LogEntry e;
            e.time = juce::Time::getCurrentTime().toMilliseconds() / 1000.0;
            e.type = std::move (type);
            e.source = std::move (source);
            e.message = std::move (message);
            e.level = level;
            entries.push_back (std::move (e));

            if ((int) entries.size() > maxEntries)
                entries.erase (entries.begin(), entries.begin() + (entries.size() - (size_t) maxEntries));
        }

        const std::vector<LogEntry>& getEntries() const noexcept { return entries; }

        void clear() { entries.clear(); }

    private:
        static constexpr int maxEntries = 500;
        std::vector<LogEntry> entries;
    };
}
