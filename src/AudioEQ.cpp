#include "../include/lib/dsp/AudioEQ.hpp"

#include <cmath>
#include <complex>

constexpr double pi = 3.14159265358979323846;

std::optional<std::string> lib::dsp::eq::lowCut(core::AudioContainer& audioContainer,
                                  const float cutoffFreq,
                                  const uint8_t steepness,
                                  float Q,
                                  const FilterType filterType) {
    // 1. Validation
    if (audioContainer.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    if (cutoffFreq < 20.0f || cutoffFreq > 20000.0f) {
        return "Target frequency should be in range between 20Hz and 20000Hz.";
    }

    if (steepness % 12 != 0 || steepness > 48 || steepness == 0) {
        return "Invalid steepness value. Available: 12, 24, 36, 48.";
    }

    if (Q < 0.025f || Q > 40.0f) {
        return "Invalid Q value. Available range is [0.025, 40].";
    }

    const auto fs = static_cast<float>(audioContainer.sampleRate_);
    if (cutoffFreq >= fs / 2.0f) {
        return "Error! Cutoff frequency exceeds Nyquist limit (fs / 2).";
    }

    const uint8_t S = steepness / 12;

    // Compute Q values for each Biquad stage
    std::vector<float> Qs(S);
    if (filterType == FilterType::Butterworth) {
        if (S == 1) {
            Qs[0] = Q; // Custom user Q for single 12 dB/oct stage
        } else {
            for (size_t k = 0; k < S; ++k) {
                Qs[k] = 1.0f / (2.0f * std::cos((2.0f * static_cast<float>(k) + 1.0f) * pi / (4.0f * static_cast<float>(S))));
            }
        }
    } else if (filterType == FilterType::LinkwitzRiley) {
        if (steepness % 24 != 0) {
            return "Error! Linkwitz-Riley requires steepness to be 24 dB/oct or 48 dB/oct.";
        }
        for (auto& q : Qs) {
            q = 0.7071067811865475f; // Fixed 1 / sqrt(2) per stage
        }
    }

    // Calculate normalized lowCut biquad coefficients for each stage
    const float w0 = 2.0f * pi * (cutoffFreq / fs);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);

    std::vector<BiquadCoeffs> stageCoeffs(S);
    for (size_t k = 0; k < S; ++k) {
        const float alpha = sin_w0 / (2.0f * Qs[k]);

        const float b0 = (1.0f + cos_w0) / 2.0f;
        const float b1 = -(1.0f + cos_w0);
        const float b2 = (1.0f + cos_w0) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * cos_w0;
        const float a2 = 1.0f - alpha;

        // Normalize by a0
        stageCoeffs[k] = {
            b0 / a0,
            b1 / a0,
            b2 / a0,
            a1 / a0,
            a2 / a0
        };
    }

    // Allocate per-channel, per-stage memory registers (Direct form II transposed)
    const size_t numChannels = audioContainer.channels_.size();
    const size_t frameCount = audioContainer.channels_[0].data_.size();
    std::vector<std::vector<StageState>> states(numChannels, std::vector<StageState>(S));

    // In-place sample processing loop
    for (size_t ch = 0; ch < numChannels; ++ch) {
        auto& channelData = audioContainer.channels_[ch].data_;

        for (size_t i = 0; i < frameCount; ++i) {
            float x = channelData[i];

            for (size_t s = 0; s < S; ++s) {
                const auto& c = stageCoeffs[s];
                auto& st = states[ch][s];

                const float y = c.b0 * x + st.z1;
                st.z1 = c.b1 * x - c.a1 * y + st.z2;
                st.z2 = c.b2 * x - c.a2 * y;

                // Flush denormals to avoid CPU microcode stalls
                if (std::abs(st.z1) < 1e-15f) st.z1 = 0.0f;
                if (std::abs(st.z2) < 1e-15f) st.z2 = 0.0f;

                x = y; // Feed stage output into next stage input
            }

            channelData[i] = x; // Overwrite sample in-place
        }
    }

    return std::nullopt;
}

std::optional<std::string> lib::dsp::eq::highCut(core::AudioContainer& audioContainer,
                                   const float cutoffFreq,
                                   const uint8_t steepness,
                                   float Q,
                                   const FilterType filterType) {
    // 1. Validation
    if (audioContainer.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    if (cutoffFreq < 20.0f || cutoffFreq > 20000.0f) {
        return "Target frequency should be in range between 20Hz and 20000Hz.";
    }

    if (steepness % 12 != 0 || steepness > 48 || steepness == 0) {
        return "Invalid steepness value. Available: 12, 24, 36, 48.";
    }

    if (Q < 0.025f || Q > 40.0f) {
        return "Invalid Q value. Available range is [0.025, 40].";
    }

    const auto fs = static_cast<float>(audioContainer.sampleRate_);
    if (cutoffFreq >= fs / 2.0f) {
        return "Error! Cutoff Frequency exceeds Nyquist limit (fs / 2).";
    }

    const uint8_t S = steepness / 12;

    // Compute Q values for each biquad stage
    std::vector<float> Qs(S);
    if (filterType == FilterType::Butterworth) {
        if (S == 1) {
            Qs[0] = Q; // Custom user Q for single 12 dB/oct stage
        } else {
            for (size_t k = 0; k < S; ++k) {
                Qs[k] = 1.0f / (2.0f * std::cos((2.0f * static_cast<float>(k) + 1.0f) * pi / (4.0f * static_cast<float>(S))));
            }
        }
    } else if (filterType == FilterType::LinkwitzRiley) {
        if (steepness % 24 != 0) {
            return "Error! Linkwitz-Riley requires steepness to be 24 dB/oct or 48 dB/oct.";
        }
        for (auto& q : Qs) {
            q = 0.7071067811865475f; // Fixed 1 / sqrt(2) per stage
        }
    }

    // Calculate normalized high cut biquad coefficients for each stage
    const float w0 = 2.0f * pi * (cutoffFreq / fs);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);

    std::vector<BiquadCoeffs> stageCoeffs(S);
    for (size_t k = 0; k < S; ++k) {
        const float alpha = sin_w0 / (2.0f * Qs[k]);

        const float b0 = (1.0f - cos_w0) / 2.0f;
        const float b1 = 1.0f - cos_w0;
        const float b2 = (1.0f - cos_w0) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * cos_w0;
        const float a2 = 1.0f - alpha;

        // Normalize by a0
        stageCoeffs[k] = {
            b0 / a0,
            b1 / a0,
            b2 / a0,
            a1 / a0,
            a2 / a0
        };
    }

    // Allocate per-channel, per-stage memory registers (direct form II transposed)
    const size_t numChannels = audioContainer.channels_.size();
    const size_t frameCount = audioContainer.channels_[0].data_.size();
    std::vector<std::vector<StageState>> states(numChannels, std::vector<StageState>(S));

    // In-place sample processing loop
    for (size_t ch = 0; ch < numChannels; ++ch) {
        auto& channelData = audioContainer.channels_[ch].data_;

        for (size_t i = 0; i < frameCount; ++i) {
            float x = channelData[i];

            for (size_t s = 0; s < S; ++s) {
                const auto& c = stageCoeffs[s];
                auto& st = states[ch][s];

                const float y = c.b0 * x + st.z1;
                st.z1 = c.b1 * x - c.a1 * y + st.z2;
                st.z2 = c.b2 * x - c.a2 * y;

                // Flush denormals
                if (std::abs(st.z1) < 1e-15f) st.z1 = 0.0f;
                if (std::abs(st.z2) < 1e-15f) st.z2 = 0.0f;

                x = y; // Feed stage output into next stage input
            }

            channelData[i] = x; // Overwrite sample in-place
        }
    }

    return std::nullopt;
}

std::optional<std::string> lib::dsp::eq::bell(core::AudioContainer& audioContainer,
                                const float targetFreq,
                                const double db,
                                float Q) {
    // 1. Validation
    if (audioContainer.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    if (targetFreq < 20.0f || targetFreq > 20000.0f) {
        return "Target frequency should be in range between 20Hz and 20000Hz.";
    }

    if (db < -36.0 || db > 36.0) {
        return "Invalid dB gain value. Recommended range is [-36, 36] dB.";
    }

    if (Q < 0.025f || Q > 40.0f) {
        return "Invalid Q value. Available range is [0.025, 40].";
    }

    const auto fs = static_cast<float>(audioContainer.sampleRate_);
    if (targetFreq >= fs / 2.0f) {
        return "Error! Target frequency exceeds Nyquist limit (fs / 2).";
    }

    // Scale Q factor (Q / sqrt(2))
    Q = Q / 1.4142135623730951f;

    const float w0 = 2.0f * pi * (targetFreq / fs);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);

    // Linear amplitude gain: A = 10^(dB / 40)
    const float A = std::pow(10.0f, static_cast<float>(db) / 40.0f);
    const float alpha = sin_w0 / (2.0f * Q);

    const float b0 = 1.0f + (alpha * A);
    const float b1 = -2.0f * cos_w0;
    const float b2 = 1.0f - (alpha * A);
    const float a0 = 1.0f + (alpha / A);
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - (alpha / A);

    // Normalize by a0
    const BiquadCoeffs coeffs{
        b0 / a0,
        b1 / a0,
        b2 / a0,
        a1 / a0,
        a2 / a0
    };

    const size_t numChannels = audioContainer.channels_.size();
    const size_t frameCount = audioContainer.channels_[0].data_.size();
    std::vector<StageState> states(numChannels);

    for (size_t ch = 0; ch < numChannels; ++ch) {
        auto& channelData = audioContainer.channels_[ch].data_;
        auto& st = states[ch];

        for (size_t i = 0; i < frameCount; ++i) {
            const float x = channelData[i];

            const float y = coeffs.b0 * x + st.z1;
            st.z1 = coeffs.b1 * x - coeffs.a1 * y + st.z2;
            st.z2 = coeffs.b2 * x - coeffs.a2 * y;

            // Flush denormals
            if (std::abs(st.z1) < 1e-15f) st.z1 = 0.0f;
            if (std::abs(st.z2) < 1e-15f) st.z2 = 0.0f;

            channelData[i] = y; // Overwrite sample in-place
        }
    }

    return std::nullopt;
}

