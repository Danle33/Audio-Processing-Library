#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

namespace lib::core {

    class AudioChannel {
    public:
        std::vector<float> data_;

        [[nodiscard]] size_t size() const { return data_.size(); }
        void resize(const size_t size) { data_.resize(size); }
    };

}