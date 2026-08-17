#include "../include/lib/dsp/Delay.hpp"
#include <cmath>
#include <algorithm>

namespace lib::dsp {

    size_t Delay::calculateTailSamples(const double delayTimeMs,
                                       const double feedback,
                                       const double mix,
                                       const uint32_t sampleRate,
                                       const double thresholdDb) const {
        if (feedback <= 0.0 || mix <= 0.0) {
            return static_cast<size_t>((delayTimeMs / 1000.0) * sampleRate);
        }

        const double epsilon = std::pow(10.0, thresholdDb / 20.0);
        if (mix <= epsilon) {
            return 0;
        }

        // Calculate N repeats until amplitude drops below epsilon
        const double n = 1.0 + (std::log(epsilon / mix) / std::log(feedback));
        const size_t repeats = static_cast<size_t>(std::ceil(std::max(1.0, n)));
        const auto delaySamplesPerTap = static_cast<size_t>((delayTimeMs / 1000.0) * sampleRate);

        return repeats * delaySamplesPerTap;
    }

    std::optional<std::string> Delay::process(core::AudioContainer& container,
                                              const double delayTimeMs,
                                              const double feedback,
                                              const double mix,
                                              const bool pingPong,
                                              const bool extendTail) {
        // Validation
        if (container.isEmpty()) {
            return "Error: AudioContainer is empty.";
        }
        if (delayTimeMs <= 0.0) {
            return "Error: Delay time must be greater than 0 ms.";
        }
        if (feedback < 0.0 || feedback >= 1.0) {
            return "Error: Feedback must be in range [0, 1).";
        }
        if (mix < 0.0 || mix > 1.0) {
            return "Error: Mix must be in range [0, 1].";
        }

        const uint32_t sampleRate = container.sampleRate_;
        const auto delaySamples = static_cast<size_t>((delayTimeMs / 1000.0) * sampleRate);

        if (delaySamples == 0) {
            return "Error: Delay time too short for current sample rate.";
        }


        if (pingPong && container.numChannels_ == 1) {
            // Mono ping pong extending
            container.numChannels_ = 2;
            container.channels_.resize(2);
            container.channels_[1].data_ = container.channels_[0].data_;
        }

        if (extendTail && feedback > 0.0 && mix > 0.0) {
            const size_t tailSamples = calculateTailSamples(delayTimeMs, feedback, mix, sampleRate);
            const size_t originalFrames = container.channels_[0].data_.size();
            const size_t extendedFrames = originalFrames + tailSamples;

            for (size_t ch = 0; ch < container.numChannels_; ++ch) {
                container.channels_[ch].data_.resize(extendedFrames, 0.0f);
            }
        }

        const size_t numChannels = container.numChannels_;
        const size_t totalFrames = container.channels_[0].data_.size();
        const size_t bufferSize = delaySamples + 1;

        if (!pingPong) {
            for (size_t ch = 0; ch < numChannels; ++ch) {
                auto& channelData = container.channels_[ch].data_;
                std::vector<float> delayBuffer(bufferSize, 0.0f);
                size_t writeIdx = 0;

                for (size_t i = 0; i < totalFrames; ++i) {
                    const float drySample = channelData[i];

                    const size_t readIdx = (writeIdx + bufferSize - delaySamples) % bufferSize;
                    const float delayedSample = delayBuffer[readIdx];

                    delayBuffer[writeIdx] = drySample + static_cast<float>(feedback) * delayedSample;

                    channelData[i] = (1.0f - static_cast<float>(mix)) * drySample
                                   + static_cast<float>(mix) * delayedSample;

                    writeIdx = (writeIdx + 1) % bufferSize;
                }
            }
        }
        else {
            auto& leftData = container.channels_[0].data_;
            auto& rightData = container.channels_[1].data_;

            std::vector<float> bufferL(bufferSize, 0.0f);
            std::vector<float> bufferR(bufferSize, 0.0f);
            size_t writeIdx = 0;

            for (size_t i = 0; i < totalFrames; ++i) {
                const float dryL = leftData[i];
                const float dryR = rightData[i];

                const float dryMono = 0.5f * (dryL + dryR);

                const size_t readIdx = (writeIdx + bufferSize - delaySamples) % bufferSize;
                const float delayedL = bufferL[readIdx];
                const float delayedR = bufferR[readIdx];

                bufferL[writeIdx] = dryMono + static_cast<float>(feedback) * delayedR;
                bufferR[writeIdx] = 0.0f + static_cast<float>(feedback) * delayedL;

                leftData[i] = (1.0f - static_cast<float>(mix)) * dryL + static_cast<float>(mix) * delayedL;
                rightData[i] = (1.0f - static_cast<float>(mix)) * dryR + static_cast<float>(mix) * delayedR;

                writeIdx = (writeIdx + 1) % bufferSize;
            }
        }

        return std::nullopt;
    }

}