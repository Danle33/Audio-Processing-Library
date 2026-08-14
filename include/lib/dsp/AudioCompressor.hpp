#pragma once
#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"


namespace lib::dsp::compressor {

    // Basic, non-saturating compressor
    // Threshold is measured in dBs
    // Attack is measured in ms, range: [0.025ms, 200ms]
    // Release is measured in ms, range: [1ms, 2000ms]
    // Lookahead is measured in ms, range: [0ms, 10ms]
    // Returns average gain reduction via gainReduction argument
    std::optional<std::string> compress(lib::core::AudioContainer& container, double threshold, double ratio,
                                            double attack, double release, double lookahead, double* gainReduction);
}
