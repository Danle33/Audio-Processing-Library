#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp::eq {
    enum FilterType {
        Butterworth, // Default, for user requested cuts
        LinkwitzRiley // Used for crossovering, flat response
    };

    struct BiquadCoeffs {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    struct StageState {
        float z1 = 0.0f;
        float z2 = 0.0f;
    };

    // High-pass filter
    // Q range: [0.025, 40]
    // Q=1: flat response
    std::optional<std::string> lowCut(core::AudioContainer& audioContainer, float cutoffFreq, uint8_t steepness, float Q,
                                    FilterType filterType = Butterworth);

    // Low-pass filter
    // Q range: [0.025, 40]
    // Q=1: flat response
    std::optional<std::string> highCut(core::AudioContainer& audioContainer, float cutoffFreq, uint8_t steepness, float Q,
                                    FilterType filterType = Butterworth);

    // Bell type boosting/cutting
    // Q range: [0.025, 40]
    // Q=0.025: wide bell
    // Q=40: narrow bell, used for surgical boosting/cutting
    // Q=1: average bell width
    std::optional<std::string> bell(core::AudioContainer& audioContainer,
                                float targetFreq,
                                double db,
                                float Q);

}