#include "../include/lib/dsp/AudioEQ.hpp"

#include <cmath>

std::optional<std::string> lib::dsp::eq::lowCut(core::AudioContainer& audioContainer, const float cutoffFreq, const uint8_t steepness, float Q,
                                                FilterType filterType) {

    if (audioContainer.channels_.empty() || audioContainer.channels_[0].data_.empty())
        return "Error! Audio container is empty.";

    if (cutoffFreq < 20.0f || cutoffFreq > 20000.0f) return "Target frequency should be in range between 20Hz and 20000Hz.";

    if (steepness % 12 != 0 || steepness > 48 || steepness == 0)
        return "Invalid steepness value. Available: 12, 24, 36, 48.";

    if (Q < 0.025f || Q > 40.0f)
        return "Invalid Q value. Available range is [0.025, 40].";

    Q = Q / std::sqrt(2.0f);
    uint8_t S = steepness / 12;
    const auto fs = audioContainer.sampleRate_;

    if (cutoffFreq >= fs / 2.0f) return "Error! Cutoff Frequency is too high.";

    std::vector<float> Qs(S);
    if (filterType == Butterworth) {
        if (S == 1) {
            Qs[0] = Q;
        }
        else {

        }
    }
    else if (filterType == LinkwitzRiley) {
        if (steepness % 24 != 0) return "Error! Linkwitz-riley requires steepness to be 24 db/oct or 48 db/oct.";

        for (auto& q : Qs)
            q = 1 / std::sqrt(2.0f);
    }


    return std::nullopt;
}

