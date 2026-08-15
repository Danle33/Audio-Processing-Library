#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

// General audio processing
namespace lib::dsp {

    // Converts stereo container into a mono one
    std::optional<std::string> stereoToMono(lib::core::AudioContainer& container);
    float calcCorrelationCoefficient(const float* samplesLeft, const float* samplesRight, size_t frameCount);

    // Trims a container, only keeps [offset, offset + duration] range, where offset and duration are measured in seconds
    std::optional<std::string> trim(lib::core::AudioContainer& container,
                                    double offset, double duration);

    // Multiplies a signal by a provided gain factor
    std::optional<std::string> volumeGain(lib::core::AudioContainer& container, double gain);

    // Adds dB amount provided to the signal
    std::optional<std::string> volumeDecibels(lib::core::AudioContainer& container, double dB);

    // Measures a global peak in dB and returns it via peak argument
    std::optional<std::string> measurePeak(const lib::core::AudioContainer& container, double* peak);

    // Joins all containers into an output container
    // Method tries to be as general as possible, resampling all sample rates to match value from the first container of the list
    std::optional<std::string> joinSignals(core::AudioContainer& outputContainer, std::vector<lib::core::AudioContainer>& containers);
}
