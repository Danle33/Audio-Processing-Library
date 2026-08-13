#pragma once

#include <atomic>
#include <vector>

#include "AudioChannel.hpp"

namespace lib::core {

    class AudioContainer {
    public:
        size_t numChannels_;
        std::vector<AudioChannel> channels_;
        uint32_t sampleRate_;

        [[nodiscard]] bool isEmpty() const {
            return channels_.empty() || channels_[0].data_.empty();
        }

    };

}
