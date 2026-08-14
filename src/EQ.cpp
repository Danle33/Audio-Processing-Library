#include "../include/lib/dsp/EQ.hpp"
#include "../include/lib/core/Constants.hpp"

#include <cmath>

namespace lib::dsp {

void BiquadStage::prepare(const size_t numChannels) {
    states_.assign(numChannels, StageState{});
}

void BiquadStage::reset() {
    for (auto& st : states_) {
        st.z1 = 0.0f;
        st.z2 = 0.0f;
    }
}

void Equalizer::reset() {
    for (auto& stage : stages_) {
        stage.reset();
    }
}

std::optional<std::string> Equalizer::validateCommonInputs(const core::AudioContainer& audioContainer,
                                                            const float frequency,
                                                            const float Q) const {
    if (audioContainer.isEmpty()) {
        return "Error! Audio container is empty.";
    }

    if (frequency < 20.0f || frequency > 20000.0f) {
        return "Target frequency should be in range between 20Hz and 20000Hz.";
    }

    if (Q < 0.025f || Q > 40.0f) {
        return "Invalid Q value. Available range is [0.025, 40].";
    }

    const auto fs = static_cast<float>(audioContainer.sampleRate_);
    if (frequency >= fs / 2.0f) {
        return "Error! Target frequency exceeds Nyquist limit (fs / 2).";
    }

    return std::nullopt;
}

void Equalizer::processContainer(core::AudioContainer& audioContainer) {
    const size_t numChannels = audioContainer.channels_.size();
    const size_t frameCount = audioContainer.channels_[0].data_.size();

    // Prepare channel delay states if container configuration changed
    if (audioContainer.sampleRate_ != lastSampleRate_ || numChannels != lastNumChannels_) {
        lastSampleRate_ = audioContainer.sampleRate_;
        lastNumChannels_ = numChannels;
        for (auto& stage : stages_) {
            stage.prepare(numChannels);
        }
    }

    for (size_t ch = 0; ch < numChannels; ++ch) {
        auto& channelData = audioContainer.channels_[ch].data_;

        for (size_t i = 0; i < frameCount; ++i) {
            float x = channelData[i];

            for (auto& stage : stages_) {
                x = stage.processSample(x, ch);
            }

            channelData[i] = x;
        }
    }
}

std::optional<std::string> Equalizer::lowCut(core::AudioContainer& audioContainer,
                                             const float cutoffFreq,
                                             const uint8_t steepness,
                                             float Q,
                                             const FilterType filterType) {
    if (auto err = validateCommonInputs(audioContainer, cutoffFreq, Q)) return err;

    if (steepness % 12 != 0 || steepness > 48 || steepness == 0) {
        return "Invalid steepness value. Available: 12, 24, 36, 48.";
    }

    const uint8_t S = steepness / 12;
    const double pi = core::constants::pi;
    const auto fs = static_cast<float>(audioContainer.sampleRate_);

    // Compute Q values for each stage
    std::vector<float> Qs(S);
    if (filterType == FilterType::Butterworth) {
        if (S == 1) {
            // Scale user Q so Q=1 yields standard Butterworth Q = 1 / sqrt(2)
            Qs[0] = Q / 1.4142135623730951f;
        } else {
            for (size_t k = 0; k < S; ++k) {
                Qs[k] = 1.0f / (2.0f * std::cos((2.0f * static_cast<float>(k) + 1.0f) * pi / (4.0f * static_cast<float>(S))));
            }
        }
    }
    else if (filterType == FilterType::LinkwitzRiley) {
        if (steepness % 24 != 0) {
            return "Error! Linkwitz-Riley requires steepness to be 24 dB/oct or 48 dB/oct.";
        }
        for (auto& q : Qs) {
            q = 0.7071067811865475f;
        }
    }

    const float w0 = 2.0f * static_cast<float>(pi) * (cutoffFreq / fs);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);

    stages_.resize(S);
    for (size_t k = 0; k < S; ++k) {
        const float alpha = sin_w0 / (2.0f * Qs[k]);

        const float b0 = (1.0f + cos_w0) / 2.0f;
        const float b1 = -(1.0f + cos_w0);
        const float b2 = (1.0f + cos_w0) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * cos_w0;
        const float a2 = 1.0f - alpha;

        stages_[k].setCoefficients({ b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 });
    }

    processContainer(audioContainer);
    return std::nullopt;
}

std::optional<std::string> Equalizer::highCut(core::AudioContainer& audioContainer,
                                              const float cutoffFreq,
                                              const uint8_t steepness,
                                              float Q,
                                              const FilterType filterType) {
    if (auto err = validateCommonInputs(audioContainer, cutoffFreq, Q)) return err;

    if (steepness % 12 != 0 || steepness > 48 || steepness == 0) {
        return "Invalid steepness value. Available: 12, 24, 36, 48.";
    }

    const uint8_t S = steepness / 12;
    const double pi = core::constants::pi;
    const auto fs = static_cast<float>(audioContainer.sampleRate_);

    std::vector<float> Qs(S);
    if (filterType == FilterType::Butterworth) {
        if (S == 1) {
            // Scale user Q so Q=1 yields standard Butterworth Q = 1 / sqrt(2)
            Qs[0] = Q / 1.4142135623730951f;
        } else {
            for (size_t k = 0; k < S; ++k) {
                Qs[k] = 1.0f / (2.0f * std::cos((2.0f * static_cast<float>(k) + 1.0f) * pi / (4.0f * static_cast<float>(S))));
            }
        }
    }
    else if (filterType == FilterType::LinkwitzRiley) {
        if (steepness % 24 != 0) {
            return "Error! Linkwitz-Riley requires steepness to be 24 dB/oct or 48 dB/oct.";
        }
        for (auto& q : Qs) {
            q = 0.7071067811865475f;
        }
    }

    const float w0 = 2.0f * static_cast<float>(pi) * (cutoffFreq / fs);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);

    stages_.resize(S);
    for (size_t k = 0; k < S; ++k) {
        const float alpha = sin_w0 / (2.0f * Qs[k]);

        const float b0 = (1.0f - cos_w0) / 2.0f;
        const float b1 = 1.0f - cos_w0;
        const float b2 = (1.0f - cos_w0) / 2.0f;
        const float a0 = 1.0f + alpha;
        const float a1 = -2.0f * cos_w0;
        const float a2 = 1.0f - alpha;

        stages_[k].setCoefficients({ b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 });
    }

    processContainer(audioContainer);
    return std::nullopt;
}

std::optional<std::string> Equalizer::bell(core::AudioContainer& audioContainer,
                                           const float targetFreq,
                                           const double dB,
                                           float Q) {
    if (auto err = validateCommonInputs(audioContainer, targetFreq, Q)) return err;

    if (dB < -36.0 || dB > 36.0) {
        return "Invalid dB gain value. Recommended range is [-36, 36] dB.";
    }

    const auto fs = static_cast<float>(audioContainer.sampleRate_);
    const double pi = core::constants::pi;

    // Scale Q factor (Q / sqrt(2))
    Q = Q / 1.4142135623730951f;

    const float w0 = 2.0f * static_cast<float>(pi) * (targetFreq / fs);
    const float cos_w0 = std::cos(w0);
    const float sin_w0 = std::sin(w0);

    const float A = std::pow(10.0f, static_cast<float>(dB) / 40.0f);
    const float alpha = sin_w0 / (2.0f * Q);

    const float b0 = 1.0f + (alpha * A);
    const float b1 = -2.0f * cos_w0;
    const float b2 = 1.0f - (alpha * A);
    const float a0 = 1.0f + (alpha / A);
    const float a1 = -2.0f * cos_w0;
    const float a2 = 1.0f - (alpha / A);

    stages_.resize(1);
    stages_[0].setCoefficients({ b0 / a0, b1 / a0, b2 / a0, a1 / a0, a2 / a0 });

    processContainer(audioContainer);
    return std::nullopt;
}

} // namespace lib::dsp