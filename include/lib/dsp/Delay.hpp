#pragma once
#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {
    class Delay {
    public:

        // Measured in seconds, prevents infinite delay
        static constexpr uint16_t MAX_CONTAINER_LENGTH = 60;

        // Feedback factor range: [0, 1], 0-one time echo, 1-infinite repeats
        // Mix factor range: [0, 1], 0-dry, 1-completely wet
        std::optional<std::string> process(core::AudioContainer& container,
                                           double delayTimeMs,
                                           double feedback,
                                           double mix,
                                           bool pingpong = false);
    };
}
