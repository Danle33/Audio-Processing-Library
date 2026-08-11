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

    };

}
