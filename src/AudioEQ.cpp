#include "../include/lib/dsp/AudioEQ.hpp"

std::optional<std::string> lib::dsp::eq::lowCut(core::AudioContainer& audioContainer, const float targetFreq, const uint8_t steepness, const float Q) {
    if (audioContainer.channels_.empty() || audioContainer.channels_[0].data_.empty())
        return "Error! Audio container is empty.";

    if (targetFreq < 20.0f || targetFreq > 20000.0f) return "Target frequency should be between 20Hz and 20000Hz.";

    if (steepness % 12 != 0 || steepness > 48)
        return "Invalid steepness value. Available: 12, 24, 36, 48.";

    if (Q < 0.0f || Q > 1.0f)
        return "Invalid Q value. Available range is [0, 1].";

    uint8_t S = steepness / 12;

    return std::nullopt;
}

std::optional<std::string> lib::dsp::eq::highCut(core::AudioContainer& audioContainer, const float targetFreq, const uint8_t steepness, const float Q) {
    if (audioContainer.channels_.empty() || audioContainer.channels_[0].data_.empty())
        return "Error! Audio container is empty.";

    if (targetFreq < 20.0f || targetFreq > 20000.0f) return "Target frequency should be between 20Hz and 20000Hz.";

    if (steepness % 12 != 0 || steepness > 48)
        return "Invalid steepness value. Available: 12, 24, 36, 48.";

    if (Q < 0.0f || Q > 1.0f)
        return "Invalid Q value. Available range is [0, 1].";

    return std::nullopt;
}

