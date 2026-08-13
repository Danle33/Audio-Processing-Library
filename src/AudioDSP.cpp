#include "../include/lib/dsp/AudioDSP.hpp"

#include <algorithm>
#include <cmath>
#include <cmath>
#include <complex>

#include "../include/lib/dsp/AudioEQ.hpp"

std::optional<std::string> lib::dsp::stereoToMono(lib::core::AudioContainer& container) {
    if (container.numChannels_ != 2 || container.channels_.size() < 2) {
        return "Audio container must be stereo (2 channels).";
    }

    // Step 1: Measure correlation between channels
    // Step 2: Split at 120Hz and hard sum the sub bass
    // Step 3: Sum the highs and multiply them by a gain factor which is dependent on ro
    // Step 4: Final mono = MonoSub + MonoHighs

    const size_t totalFrames = container.channels_[0].data_.size();
    if (totalFrames == 0) {
        return "Audio container is empty.";
    }

    core::AudioContainer lowBand = container;
    core::AudioContainer highBand = container;

    eq::highCut(lowBand, 120.0f, 24, 1.0f, eq::FilterType::LinkwitzRiley);
    eq::lowCut(highBand, 120.0f, 24, 1.0f, eq::FilterType::LinkwitzRiley);

    const size_t windowSize = static_cast<size_t>(container.sampleRate_ * 0.020f); // 20 ms window
    float roPrev = 1.0f;
    const float alphaSmooth = 0.99f;

    for (size_t blockStart = 0; blockStart < totalFrames; blockStart += windowSize) {
        // Handle partial block at the end of the file
        const size_t currentBlockSize = std::min(windowSize, totalFrames - blockStart);

        const float* samplesLeft = container.channels_[0].data_.data() + blockStart;
        const float* samplesRight = container.channels_[1].data_.data() + blockStart;

        float ro = calcCorrelationCoefficient(samplesLeft, samplesRight, currentBlockSize);

        // Temporal smoothing
        if (blockStart > 0) {
            ro = alphaSmooth * roPrev + (1.0f - alphaSmooth) * ro;
        }
        roPrev = ro;

        // Safeguard against out of phase sqrt(0) / division by zero
        const float safeRo = std::max(0.0f, ro);
        const float gain = 1.0f / std::sqrt(2.0f + 2.0f * safeRo);

        for (size_t k = 0; k < currentBlockSize; ++k) {
            const size_t idx = blockStart + k;

            const float monoSub = 0.5f * (lowBand.channels_[0].data_[idx] + lowBand.channels_[1].data_[idx]);
            const float monoHigh = gain * (highBand.channels_[0].data_[idx] + highBand.channels_[1].data_[idx]);

            container.channels_[0].data_[idx] = monoSub + monoHigh;
        }
    }

    // Convert container to a mono structure
    container.numChannels_ = 1;
    container.channels_.resize(1);

    return std::nullopt;
}

float lib::dsp::calcCorrelationCoefficient(const float* samplesLeft, const float* samplesRight, const size_t frameCount) {
    if (frameCount == 0) return 1.0f;

    double dotProduct = 0.0;
    double energyLeft = 0.0;
    double energyRight = 0.0;

    for (size_t i = 0; i < frameCount; ++i) {
        const auto l = static_cast<double>(samplesLeft[i]);
        const auto r = static_cast<double>(samplesRight[i]);

        dotProduct  += l * r;
        energyLeft  += l * l;
        energyRight += r * r;
    }

    // Silence guard
    const double energyProduct = energyLeft * energyRight;
    if (energyProduct < 1e-9) {
        return 1.0f;
    }

    const double ro = dotProduct / std::sqrt(energyProduct);
    return std::clamp(static_cast<float>(ro), -1.0f, 1.0f);
}

std::optional<std::string> lib::dsp::trim(lib::core::AudioContainer& container, const double offset, const double duration) {
    if (container.isEmpty()) return "Error! Audio container is empty.";
    if (offset < 0 || duration < 0) return "Error! Offset and duration should be non negative values.";

    const auto sampleCount = container.channels_[0].data_.size();
    const auto bufferDuration = static_cast<double>(sampleCount) / container.sampleRate_;

    if (offset >= bufferDuration) return "Offset exceeds buffer length.";

    core::AudioContainer trimmedContainer;
    trimmedContainer.sampleRate_ = container.sampleRate_;
    trimmedContainer.numChannels_ = container.numChannels_;
    trimmedContainer.channels_.resize(container.numChannels_);

    auto start = static_cast<size_t>(offset * container.sampleRate_);
    auto end = static_cast<size_t>((offset + duration) * container.sampleRate_);
    end = std::min(end, static_cast<size_t>(bufferDuration * container.sampleRate_));

    trimmedContainer.channels_[0].resize(static_cast<size_t>(end - start + 1));
    trimmedContainer.channels_[1].resize(static_cast<size_t>(end - start + 1));

    for (size_t i = 0; i < end - start + 1; i++) {
        trimmedContainer.channels_[0].data_[i] = container.channels_[0].data_[i + start];
        trimmedContainer.channels_[1].data_[i] = container.channels_[1].data_[i + start];
    }

    container.channels_.clear();
    container = trimmedContainer;

    return std::nullopt;
}

std::optional<std::string> lib::dsp::volumeGain(lib::core::AudioContainer& container, const double gain) {
    if (container.isEmpty()) return "Error! Audio container is empty.";
    if (gain < 0) return "Gain factor should be a positive value.";

    for (size_t ch = 0; ch < container.numChannels_; ++ch) {
        for (size_t i = 0; i < container.channels_[ch].size(); ++i) {
            container.channels_[ch].data_[i] *= gain;
        }
    }

    return std::nullopt;
}

std::optional<std::string> lib::dsp::volumeDecibels(lib::core::AudioContainer& container, const double dB) {
    return volumeGain(container, std::pow(10, dB / 20));
}

std::optional<std::string> lib::dsp::measurePeak(const lib::core::AudioContainer& container, double* peak) {
    if (!peak) {
        return "Error! Null pointer provided for peak output.";
    }

    if (container.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    float maxLinearPeak = 0.0f;

    for (size_t ch = 0; ch < container.numChannels_; ++ch) {
        const auto& channelData = container.channels_[ch].data_;
        const size_t frameCount = channelData.size();

        for (size_t i = 0; i < frameCount; ++i) {
            maxLinearPeak = std::max(maxLinearPeak, std::abs(channelData[i]));
        }
    }

    // Noise floor clamp
    const double safePeak = std::max(static_cast<double>(maxLinearPeak), 1e-6);

    *peak = 20.0 * std::log10(safePeak);

    return std::nullopt;
}



