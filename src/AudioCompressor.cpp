

#include <optional>
#include <string>
#include "../include/lib/core/AudioContainer.hpp"
#include "../include/lib/dsp/AudioCompressor.hpp"

#include <cmath>

std::optional<std::string> lib::dsp::compressor::compress(lib::core::AudioContainer& container,
                                                          const double threshold,
                                                          const double ratio,
                                                          const double attack,
                                                          const double release,
                                                          const double lookahead,
                                                          double* gainReduction) {
    // 1. Input Validation
    if (container.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    if (container.sampleRate_ == 0) {
        return "Error! Invalid sample rate.";
    }

    if (threshold > 0.0 || threshold < -100.0) {
        return "Invalid threshold value. Expected range is [-100, 0] dB.";
    }

    if (ratio < 1.0 || ratio > 100.0) {
        return "Invalid ratio value. Expected range is [1, 100].";
    }

    if (attack < 0.025 || attack > 200.0) {
        return "Invalid attack time. Expected range is [0.025, 200] ms.";
    }

    if (release < 1.0 || release > 2000.0) {
        return "Invalid release time. Expected range is [1, 2000] ms.";
    }

    if (lookahead < 0.0 || lookahead > 10.0) {
        return "Invalid lookahead time. Expected range is [0, 10] ms.";
    }

    // 2. Precompute Coefficients
    const auto fs = static_cast<double>(container.sampleRate_);
    const double attackSec = attack / 1000.0;
    const double releaseSec = release / 1000.0;
    const double lookaheadSec = lookahead / 1000.0;

    const double alphaAtt = 1.0 - std::exp(-1.0 / (fs * attackSec));
    const double alphaRel = 1.0 - std::exp(-1.0 / (fs * releaseSec));

    const auto lookaheadSamples = static_cast<size_t>(std::round(fs * lookaheadSec));

    const size_t numChannels = container.numChannels_;
    const size_t frameCount = container.channels_[0].data_.size();

    double g_dB = 0.0;
    double totalGainReduction_dB = 0.0;

    // 3. Sample-by-Sample Loop with Lookahead Offset
    for (size_t i = 0; i < frameCount; ++i) {
        // Lookahead detector index reads D samples ahead of current output frame i
        const size_t detectorIdx = i + lookaheadSamples;

        float peak = 0.0f;
        if (detectorIdx < frameCount) {
            for (size_t ch = 0; ch < numChannels; ++ch) {
                peak = std::max(peak, std::abs(container.channels_[ch].data_[detectorIdx]));
            }
        }

        // Noise floor clamp to prevent log10(0) -> -infinity
        const double peakClamped = std::max(static_cast<double>(peak), 1e-6);
        const double peak_dB = 20.0 * std::log10(peakClamped);

        const double overshoot_dB = peak_dB - threshold;
        double targetGain_dB = 0.0;

        if (overshoot_dB > 0.0) {
            targetGain_dB = -overshoot_dB + overshoot_dB * (1.0 / ratio);
        }

        if (targetGain_dB < g_dB) {
            g_dB += alphaAtt * (targetGain_dB - g_dB);
        } else {
            g_dB += alphaRel * (targetGain_dB - g_dB);
        }

        totalGainReduction_dB += std::abs(g_dB);

        const auto gainLin = static_cast<float>(std::pow(10.0, g_dB / 20.0));

        for (size_t ch = 0; ch < numChannels; ++ch) {
            container.channels_[ch].data_[i] *= gainLin;
        }
    }

    if (gainReduction) {
        *gainReduction = totalGainReduction_dB / static_cast<double>(frameCount);
    }

    return std::nullopt;
}
