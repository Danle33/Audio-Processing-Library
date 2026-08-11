#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {

    // converts stereo container into a mono one
    std::optional<std::string> stereoToMono(lib::core::AudioContainer& container);

    float calcCorrelationCoefficient(const float* samplesLeft, const float* samplesRight, size_t frameCount);
}
