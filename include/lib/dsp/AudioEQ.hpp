#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp::eq {
    std::optional<std::string> lowCut(core::AudioContainer& audioContainer, float targetFreq, uint8_t steepness, float Q);
    std::optional<std::string> highCut(core::AudioContainer& audioContainer, float targetFreq, uint8_t steepness, float Q);
}