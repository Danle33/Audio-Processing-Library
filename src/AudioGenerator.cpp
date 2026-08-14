#include "../include/lib/signalGeneration/AudioGenerator.hpp"

#include <cmath>

#include "../include/lib/core/Constants.hpp"

std::optional<std::string> lib::audioGenerator::generateSineWave(lib::core::AudioContainer& container, const double frequency,
                                                                 const double amplitude, const double phase,
                                                                 const double lengthSeconds, const uint32_t sampleRate) {
    if (!container.isEmpty()) {
        container.channels_[0].data_.clear();
        if (container.numChannels_ != 1)
            container.channels_[1].data_.clear();
        container.channels_.clear();
    }

    if (frequency <= 0.0 || frequency > 20000.0) return "Frequency should be positive and less than or equal to 20000Hz";
    if (amplitude <= 0 || amplitude > 1) return "Amplitude range should be: (0, 1]";
    if (lengthSeconds <= 0) return "Length should be greater than zero";

    container.numChannels_ = 1;
    container.sampleRate_ = sampleRate;
    container.channels_.resize(1);

    const auto frameCount = static_cast<size_t>(lengthSeconds * sampleRate);

    auto& leftChannel = container.channels_[0].data_;
    leftChannel.resize(frameCount);

    double pi = core::constants::pi;
    const auto phaseRadians = phase * pi / 180;

    for (size_t i = 0; i < frameCount; ++i) {
        leftChannel[i] = amplitude * std::sin(2 * pi * frequency * i / sampleRate + phaseRadians);
    }

    return std::nullopt;
}
