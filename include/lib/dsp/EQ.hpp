#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {

    enum class FilterType {
        Butterworth,   // Standard user requested cuts
        LinkwitzRiley  // For crossovers, produces a flat magnitude response at crossover point
    };

    struct BiquadCoeffs {
        float b0 = 0.0f, b1 = 0.0f, b2 = 0.0f;
        float a1 = 0.0f, a2 = 0.0f;
    };

    struct StageState {
        float z1 = 0.0f;
        float z2 = 0.0f;
    };

    // Represents a single biquad stage with per-channel state
    class BiquadStage {
    public:
        void setCoefficients(const BiquadCoeffs& coeffs) { coeffs_ = coeffs; }
        void prepare(size_t numChannels);
        void reset();

        inline float processSample(const float x, const size_t channelIndex) {
            auto& st = states_[channelIndex];
            const float y = coeffs_.b0 * x + st.z1;
            st.z1 = coeffs_.b1 * x - coeffs_.a1 * y + st.z2;
            st.z2 = coeffs_.b2 * x - coeffs_.a2 * y;

            // Flush denormals to avoid CPU stalls
            if (std::abs(st.z1) < 1e-15f) st.z1 = 0.0f;
            if (std::abs(st.z2) < 1e-15f) st.z2 = 0.0f;

            return y;
        }

    private:
        BiquadCoeffs coeffs_{};
        std::vector<StageState> states_;
    };

    class Equalizer {
    public:
        Equalizer() = default;

        std::optional<std::string> lowCut(core::AudioContainer& audioContainer,
                                         float cutoffFreq,
                                         uint8_t steepness = 12,
                                         float Q = 1.0f,
                                         FilterType filterType = FilterType::Butterworth);

        std::optional<std::string> highCut(core::AudioContainer& audioContainer,
                                          float cutoffFreq,
                                          uint8_t steepness = 12,
                                          float Q = 1.0f,
                                          FilterType filterType = FilterType::Butterworth);

        std::optional<std::string> bell(core::AudioContainer& audioContainer,
                                       float targetFreq,
                                       double dB,
                                       float Q = 1.0f);

        // Clear filter history registers
        void reset();

    private:
        [[nodiscard]] std::optional<std::string> validateCommonInputs(const core::AudioContainer& audioContainer,
                                                         float frequency,
                                                         float Q) const;

        void processContainer(core::AudioContainer& audioContainer);

        std::vector<BiquadStage> stages_;
        uint32_t lastSampleRate_{0};
        size_t lastNumChannels_{0};
    };

} // namespace lib::dsp