#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "../core/AudioContainer.hpp"

namespace lib::dsp {

    enum class InterpolationType {
        Linear,  // Simple and fast, but low quality
        Cubic,   // Hermite cubic spline (Default - fast and high quality)
        Sinc     // Blackman-windowed Sinc (Highest quality, brickwall anti-aliasing)
    };

    class Resampler {
    public:
        Resampler() = default;

        // Resamples container to targetSampleRate in-place
        std::optional<std::string> process(lib::core::AudioContainer& container,
                                           uint32_t targetSampleRate,
                                           InterpolationType type = InterpolationType::Cubic);

        friend class PitchShifter;

    private:
        static float interpolateLinear(const std::vector<float>& buffer, double samplePos);
        static float interpolateCubic(const std::vector<float>& buffer, double samplePos);
        static float interpolateSinc(const std::vector<float>& buffer,
                                double samplePos,
                                double cutoffRatio,
                                const std::vector<float>& window,
                                int kernelRadius = 16);
    };

}