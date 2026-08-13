#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {

    // converts stereo container into a mono one
    std::optional<std::string> stereoToMono(lib::core::AudioContainer& container);
    float calcCorrelationCoefficient(const float* samplesLeft, const float* samplesRight, size_t frameCount);

    // trims a container, only keeps [offset, offset + duration] range, where offset and duration are measured in seconds
    std::optional<std::string> trim(lib::core::AudioContainer& container,
                                    double offset, double duration);
}
