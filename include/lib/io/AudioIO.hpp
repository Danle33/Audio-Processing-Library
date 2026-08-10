#pragma once
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::io {
        bool read(const std::string& path, lib::core::AudioContainer& container);
        bool write(const std::string& path, lib::core::AudioContainer& container);

        bool decodeWav(std::vector<uint8_t>& buffer, lib::core::AudioContainer& container);
}
