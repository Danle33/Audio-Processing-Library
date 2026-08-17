#include "../include/lib/dsp/Delay.hpp"

std::optional<std::string> lib::dsp::Delay::process(core::AudioContainer& container, const double delayTimeMs,
                                                    const double feedback, const double mix, const bool pingpong) {
    if (container.isEmpty()) return "Error! Container is empty.";

    if (pingpong) {
        if (container.numChannels_ == 1) {
            // Extend to stereo by duplicated the only channel
            container.numChannels_ == 2;
            container.channels_.resize(2);

            container.channels_[1] = container.channels_[0];
        }
    }

    const uint32_t delayTimeSamples = delayTimeMs / 1000.0 * container.sampleRate_;

    uint32_t readPtr = 0;
    uint32_t writePtr = readPtr + delayTimeSamples;

    // TODO

    return std::nullopt;
}
