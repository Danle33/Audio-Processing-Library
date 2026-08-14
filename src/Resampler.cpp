#include "../include/lib/dsp/Resampler.hpp"
#include "../include/lib/core/Constants.hpp"

#include <algorithm>
#include <cmath>

namespace lib::dsp {

    // Linear Interpolation (Fastest)
    float Resampler::interpolateLinear(const std::vector<float>& buffer, const double samplePos) {
        const size_t size = buffer.size();
        const auto i0 = static_cast<size_t>(samplePos);
        const size_t i1 = std::min(i0 + 1, size - 1);
        const auto alpha = static_cast<float>(samplePos - i0);

        return (1.0f - alpha) * buffer[i0] + alpha * buffer[i1];
    }

    // 4-Point 3rd-Order Hermite Cubic Interpolation (Default)
    float Resampler::interpolateCubic(const std::vector<float>& buffer, const double samplePos) {
        const size_t size = buffer.size();
        const auto i1 = static_cast<size_t>(samplePos);

        const size_t i0 = (i1 > 0) ? i1 - 1 : 0;
        const size_t i2 = std::min(i1 + 1, size - 1);
        const size_t i3 = std::min(i1 + 2, size - 1);

        const float y0 = buffer[i0];
        const float y1 = buffer[i1];
        const float y2 = buffer[i2];
        const float y3 = buffer[i3];

        const auto a = static_cast<float>(samplePos - i1);

        const float c0 = y1;
        const float c1 = 0.5f * (y2 - y0);
        const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * a + c2) * a + c1) * a + c0;
    }

    // Blackman-Windowed Sinc Interpolation
    float Resampler::interpolateSinc(const std::vector<float>& buffer,
                                     const double samplePos,
                                     const double cutoffRatio,
                                     const std::vector<float>& window,
                                     const int kernelRadius) {
        const size_t size = buffer.size();
        const auto centerIdx = static_cast<int64_t>(samplePos);
        constexpr double pi = core::constants::pi;
        const float filterCutoff = static_cast<float>(std::min(1.0, cutoffRatio));

        float sum = 0.0f;
        float weightSum = 0.0f;

        for (int k = -kernelRadius; k <= kernelRadius; ++k) {
            const int64_t srcIdx = centerIdx + k;

            if (srcIdx >= 0 && srcIdx < static_cast<int64_t>(size)) {
                const double x = (samplePos - static_cast<double>(srcIdx)) * filterCutoff;

                float sincVal = 1.0f;
                if (std::abs(x) > 1e-7) {
                    const double px = pi * x;
                    sincVal = static_cast<float>(std::sin(px) / px);
                }

                const float winWeight = window[k + kernelRadius];
                const float weight = sincVal * winWeight * filterCutoff;

                sum += buffer[static_cast<size_t>(srcIdx)] * weight;
                weightSum += weight;
            }
        }

        return (weightSum > 0.0f) ? (sum / weightSum) : 0.0f;
    }

    std::optional<std::string> Resampler::process(lib::core::AudioContainer& container,
                                                  const uint32_t targetSampleRate,
                                                  const InterpolationType type) {
        if (container.isEmpty()) return "Error! Container is empty.";
        if (container.sampleRate_ == 0) return "Error! Source sample rate is 0.";
        if (targetSampleRate == 0 || targetSampleRate > 192000) return "Error! Target sample rate range is [0Hz, 192000Hz]";

        if (container.sampleRate_ == targetSampleRate) return std::nullopt;

        const auto sourceRate = static_cast<double>(container.sampleRate_);
        const auto targetRate = static_cast<double>(targetSampleRate);
        const double ratio = sourceRate / targetRate;

        if (ratio <= 0.0 || std::isnan(ratio) || std::isinf(ratio)) {
            return "Error! Invalid resampling ratio calculated.";
        }

        const size_t numChannels = container.numChannels_;
        const size_t sourceFrameCount = container.channels_[0].data_.size();
        const auto targetFrameCount = static_cast<size_t>(std::round(static_cast<double>(sourceFrameCount) / ratio));

        if (targetFrameCount == 0) return "Error! Calculated target frame count is 0.";

        const double cutoffRatio = targetRate / sourceRate;

        // Pre-compute window table once for Sinc processing
        constexpr int kernelRadius = 16;
        constexpr double pi = core::constants::pi;
        std::vector<float> blackmanWindow(2 * kernelRadius + 1);

        if (type == InterpolationType::Sinc) {
            for (int k = -kernelRadius; k <= kernelRadius; ++k) {
                const double normK = static_cast<double>(k) / static_cast<double>(kernelRadius);
                blackmanWindow[k + kernelRadius] = static_cast<float>(
                    0.42 + 0.5 * std::cos(pi * normK) + 0.08 * std::cos(2.0 * pi * normK)
                );
            }
        }

        // Process Channels
        for (size_t ch = 0; ch < numChannels; ++ch) {
            const auto& sourceData = container.channels_[ch].data_;
            std::vector<float> resampledData(targetFrameCount);

            for (size_t i = 0; i < targetFrameCount; ++i) {
                const double sourceSamplePos = static_cast<double>(i) * ratio;

                switch (type) {
                    case InterpolationType::Sinc:
                        resampledData[i] = interpolateSinc(sourceData, sourceSamplePos, cutoffRatio, blackmanWindow, kernelRadius);
                        break;
                    case InterpolationType::Cubic:
                        resampledData[i] = interpolateCubic(sourceData, sourceSamplePos);
                        break;
                    case InterpolationType::Linear:
                        resampledData[i] = interpolateLinear(sourceData, sourceSamplePos);
                        break;
                }
            }

            container.channels_[ch].data_ = std::move(resampledData);
        }

        container.sampleRate_ = targetSampleRate;
        return std::nullopt;
    }

} // namespace lib::dsp