#pragma once

#include <atomic>
#include <vector>

#include "AudioChannel.hpp"

namespace lib::core {

    class AudioContainer {
    public:
        size_t numChannels_;
        std::vector<AudioChannel> channels_; // 1 or 2 channels
        uint32_t sampleRate_;

        [[nodiscard]] bool isEmpty() const {
            return channels_.empty() || channels_[0].data_.empty();
        }

        void clear() {
            if (!isEmpty()) {
                channels_[0].data_.clear();
                if (numChannels_ != 1)
                    channels_[1].data_.clear();
                channels_.clear();
            }
        }

    };

}
