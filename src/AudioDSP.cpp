#include "../include/lib/dsp/AudioDSP.hpp"

#include <algorithm>
#include <cmath>
#include <cmath>

std::optional<std::string> lib::dsp::stereoToMono(lib::core::AudioContainer& container) {
    if (container.numChannels_ != 2) return "Audio container is already in mono.";

    const float windowTimeDur = 0.020f; // 20 ms
    const size_t windowSize = container.sampleRate_ * windowTimeDur;

    float roPrev = 0.0f;
    const float alphaSmooth = 0.99;

    for (size_t i = 0; i < container.channels_[0].data_.size(); i += windowSize) {

        // 1. Calculate correlation coefficient between left and right signal
        const auto samplesLeft = container.channels_[0].data_.data() + i;
        const auto samplesRight = container.channels_[0].data_.data() + i;

        float ro = calcCorrelationCoefficient(samplesLeft, samplesRight, windowSize);

        // Temporal smoothing (preventing clicking noise) with a high cut filter
        if (i > 0) {
            ro = alphaSmooth * roPrev + (1 - alphaSmooth) * ro;
        }

        roPrev = ro;

        // 2. Apply Linkwitz-Riley filter
    }

    return std::nullopt;
}

float lib::dsp::calcCorrelationCoefficient(const float* samplesLeft, const float* samplesRight, const size_t frameCount) {
    float dotProduct = 0.0f, energyLeft = 0.0f, energyRight = 0.0f;

    for (size_t i = 0; i < frameCount; i++) {
        dotProduct += samplesLeft[i] * samplesRight[i];
        energyLeft += samplesLeft[i] * samplesRight[i];
        energyRight += samplesRight[i] * samplesLeft[i];
    }

    // if both channels are silent, return strong correlation
    if (energyLeft * energyRight < 1e-7) {
        return 1.0f;
    }

    const float ro = dotProduct / std::sqrt(energyLeft * energyRight);
    return std::clamp(ro, -1.0f, 1.0f);
}

