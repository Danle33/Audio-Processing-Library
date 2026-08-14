#include "../include/lib/dsp/Compressor.hpp"

#include <algorithm>
#include <cmath>

namespace lib::dsp {

Compressor::Compressor(const double thresholdDb,
                       const double ratio,
                       const double attackMs,
                       const double releaseMs,
                       const double lookaheadMs)
    : thresholdDb_(thresholdDb),
      ratio_(ratio),
      attackMs_(attackMs),
      releaseMs_(releaseMs),
      lookaheadMs_(lookaheadMs) {}

std::optional<std::string> Compressor::setThreshold(const double thresholdDb) {
    if (thresholdDb > 0.0 || thresholdDb < -100.0) {
        return "Invalid threshold value. Expected range is [-100, 0] dB.";
    }
    thresholdDb_ = thresholdDb;
    return std::nullopt;
}

std::optional<std::string> Compressor::setRatio(const double ratio) {
    if (ratio < 1.0 || ratio > 100.0) {
        return "Invalid ratio value. Expected range is [1, 100].";
    }
    ratio_ = ratio;
    return std::nullopt;
}

std::optional<std::string> Compressor::setAttack(const double attackMs) {
    if (attackMs < 0.025 || attackMs > 200.0) {
        return "Invalid attack time. Expected range is [0.025, 200] ms.";
    }
    attackMs_ = attackMs;
    if (lastSampleRate_ > 0) {
        updateCoefficients(lastSampleRate_);
    }
    return std::nullopt;
}

std::optional<std::string> Compressor::setRelease(const double releaseMs) {
    if (releaseMs < 1.0 || releaseMs > 2000.0) {
        return "Invalid release time. Expected range is [1, 2000] ms.";
    }
    releaseMs_ = releaseMs;
    if (lastSampleRate_ > 0) {
        updateCoefficients(lastSampleRate_);
    }
    return std::nullopt;
}

std::optional<std::string> Compressor::setLookahead(const double lookaheadMs) {
    if (lookaheadMs < 0.0 || lookaheadMs > 10.0) {
        return "Invalid lookahead time. Expected range is [0, 10] ms.";
    }
    lookaheadMs_ = lookaheadMs;
    if (lastSampleRate_ > 0) {
        updateCoefficients(lastSampleRate_);
    }
    return std::nullopt;
}

void Compressor::reset() {
    g_dB_ = 0.0;
}

void Compressor::updateCoefficients(const uint32_t sampleRate) {
    lastSampleRate_ = sampleRate;
    const auto fs = static_cast<double>(sampleRate);

    const double attackSec = attackMs_ / 1000.0;
    const double releaseSec = releaseMs_ / 1000.0;
    const double lookaheadSec = lookaheadMs_ / 1000.0;

    alphaAtt_ = 1.0 - std::exp(-1.0 / (fs * attackSec));
    alphaRel_ = 1.0 - std::exp(-1.0 / (fs * releaseSec));
    lookaheadSamples_ = static_cast<size_t>(std::round(fs * lookaheadSec));
}

std::optional<std::string> Compressor::process(lib::core::AudioContainer& container, double* gainReduction) {
    // Validation
    if (container.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    if (container.sampleRate_ == 0) {
        return "Error! Invalid sample rate.";
    }

    if (auto err = setThreshold(thresholdDb_)) return err;
    if (auto err = setRatio(ratio_)) return err;
    if (auto err = setAttack(attackMs_)) return err;
    if (auto err = setRelease(releaseMs_)) return err;
    if (auto err = setLookahead(lookaheadMs_)) return err;

    // Precompute / update coefficients if sample rate changed
    if (container.sampleRate_ != lastSampleRate_) {
        updateCoefficients(container.sampleRate_);
    }

    const size_t numChannels = container.numChannels_;
    const size_t frameCount = container.channels_[0].data_.size();

    double totalGainReduction_dB = 0.0;

    for (size_t i = 0; i < frameCount; ++i) {
        const size_t detectorIdx = i + lookaheadSamples_;

        float peak = 0.0f;
        if (detectorIdx < frameCount) {
            for (size_t ch = 0; ch < numChannels; ++ch) {
                peak = std::max(peak, std::abs(container.channels_[ch].data_[detectorIdx]));
            }
        }

        // Noise floor clamp to prevent log10(0) = -infinity
        const double peakClamped = std::max(static_cast<double>(peak), 1e-6);
        const double peak_dB = 20.0 * std::log10(peakClamped);

        const double overshoot_dB = peak_dB - thresholdDb_;
        double targetGain_dB = 0.0;

        if (overshoot_dB > 0.0) {
            targetGain_dB = -overshoot_dB + overshoot_dB * (1.0 / ratio_);
        }

        if (targetGain_dB < g_dB_) {
            g_dB_ += alphaAtt_ * (targetGain_dB - g_dB_);
        } else {
            g_dB_ += alphaRel_ * (targetGain_dB - g_dB_);
        }

        totalGainReduction_dB += std::abs(g_dB_);

        const auto gainLin = static_cast<float>(std::pow(10.0, g_dB_ / 20.0));

        for (size_t ch = 0; ch < numChannels; ++ch) {
            container.channels_[ch].data_[i] *= gainLin;
        }
    }

    if (gainReduction) {
        *gainReduction = totalGainReduction_dB / static_cast<double>(frameCount);
    }

    return std::nullopt;
}

}