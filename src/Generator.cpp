#include "../include/lib/generator/Generator.hpp"

#include <cmath>

#include "../include/lib/core/Constants.hpp"

std::optional<std::string> lib::audioGenerator::generateSineWave(lib::core::AudioContainer& container, const double frequency,
                                                                 const double amplitude, const double phase,
                                                                 const double lengthSeconds, const uint32_t sampleRate) {
    container.clear();

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

std::optional<std::string> lib::audioGenerator::joinSineWaves(core::AudioContainer& outputContainer, std::vector<lib::core::AudioContainer>& containers) {
    outputContainer.clear();

    if (containers.empty()) return std::nullopt;

    outputContainer.numChannels_ = 1;
    outputContainer.sampleRate_ = containers[0].sampleRate_;
    outputContainer.channels_.resize(1);

    const auto frameCount = containers[0].channels_[0].data_.size();

    auto& leftChannel = outputContainer.channels_[0].data_;
    leftChannel.resize(frameCount);

    for (auto& container : containers) {
        if (container.channels_.empty()) return "Error! Vector contains and empty container.";
        if (container.channels_[0].data_.size() != frameCount) return "Error! Input container data size mismatch.";

        for (size_t j = 0; j < frameCount; ++j) {
            leftChannel[j] += container.channels_[0].data_[j];
        }
    }

    return std::nullopt;
}

