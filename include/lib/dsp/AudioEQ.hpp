#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp::eq {
    enum FilterType {
        Butterworth, // default, for user requested cuts
        LinkwitzRiley // used for crossovering, flat response
    };

    struct BiquadCoeffs {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    struct StageState {
        float z1 = 0.0f;
        float z2 = 0.0f;
    };

    std::optional<std::string> lowCut(core::AudioContainer& audioContainer, float cutoffFreq, uint8_t steepness, float Q,
                                    FilterType filterType = Butterworth);

    std::optional<std::string> highCut(core::AudioContainer& audioContainer, float cutoffFreq, uint8_t steepness, float Q,
                                    FilterType filterType = Butterworth);

}