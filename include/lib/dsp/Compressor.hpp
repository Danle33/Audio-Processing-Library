#pragma once

#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {

    class Compressor {
    public:
        explicit Compressor(double thresholdDb = -12.0,
                   double ratio = 4.0,
                   double attackMs = 10.0,
                   double releaseMs = 100.0,
                   double lookaheadMs = 0.0);

        // Process an entire container
        std::optional<std::string> process(lib::core::AudioContainer& container, double* gainReduction = nullptr);

        std::optional<std::string> setThreshold(double thresholdDb);
        std::optional<std::string> setRatio(double ratio);
        std::optional<std::string> setAttack(double attackMs);
        std::optional<std::string> setRelease(double releaseMs);
        std::optional<std::string> setLookahead(double lookaheadMs);

        [[nodiscard]] double getThreshold() const { return thresholdDb_; }
        [[nodiscard]] double getRatio() const { return ratio_; }
        [[nodiscard]] double getAttack() const { return attackMs_; }
        [[nodiscard]] double getRelease() const { return releaseMs_; }
        [[nodiscard]] double getLookahead() const { return lookaheadMs_; }

        // State Control
        void reset();

    private:
        void updateCoefficients(uint32_t sampleRate);

        // User Parameters
        double thresholdDb_;
        double ratio_;
        double attackMs_;
        double releaseMs_;
        double lookaheadMs_;

        // Internal State/DSP Coefficients
        double g_dB_{0.0};
        double alphaAtt_{0.0};
        double alphaRel_{0.0};
        size_t lookaheadSamples_{0};
        uint32_t lastSampleRate_{0};
    };

}