#pragma once

#include <juce_core/juce_core.h>

namespace vp
{
    // Runs the offline audio self-tests (no audio device required) and writes a
    // report to outputFile. Returns the number of failed checks.
    int runSelfTests (const juce::File& outputFile);
}
