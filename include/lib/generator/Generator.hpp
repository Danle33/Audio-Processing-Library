#pragma once
#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::audioGenerator {

    // Generates a pure sine wave with provided parameters
    // Frequency: (0Hz, 20000Hz)
    // Amplitude: (0, 1]
    // Phase is measured in degrees
    std::optional<std::string> generateSineWave(lib::core::AudioContainer& container, double frequency, double amplitude,
                                                double phase, double lengthSeconds, uint32_t sampleRate);

}
