#pragma once
#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {
    class Delay {
    public:

        // Measured in seconds, prevents infinite delay
        static constexpr uint16_t MAX_CONTAINER_LENGTH = 60;

        // Feedback factor range: [0, 1), 0-one time echo, 1-infinite repeats
        // Mix factor range: [0, 1), 0-dry, 1-completely wet
        std::optional<std::string> process(core::AudioContainer& container,
                                              double delayTimeMs,
                                              double feedback,
                                              double mix,
                                              bool pingPong = false,
                                              bool extendTail = true);

    private:
        [[nodiscard]] size_t calculateTailSamples(double delayTimeMs,
                                    double feedback,
                                    double mix,
                                    uint32_t sampleRate,
                                    double thresholdDb = -96.0) const;
    };
}
