#pragma once

#include <complex>
#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {

    class PitchShifter {
    public:
        explicit PitchShifter() = default;

        // Process an entire container
        std::optional<std::string> process(lib::core::AudioContainer& container, double semitones, double formantShift);
    };

}