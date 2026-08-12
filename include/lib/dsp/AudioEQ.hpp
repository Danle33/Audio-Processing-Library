#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp::eq {
    enum FilterType {
        Butterworth, // default, for user requested cuts
        LinkwitzRiley // used for crossovering, flat response
    };

    std::optional<std::string> lowCut(core::AudioContainer& audioContainer, float cutoffFreq, uint8_t steepness, float Q,
                                    FilterType filterType = Butterworth);

}