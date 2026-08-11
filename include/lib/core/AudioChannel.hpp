#pragma once

#include <cstdint>
#include <vector>

namespace lib::core {

    class AudioChannel {
    public:
        std::vector<float> data_;
    };

}