#pragma once
#include <optional>
#include <string>

#include "../core/AudioContainer.hpp"
#include "Wav.hpp"

namespace lib::io {
        // Reads data from a file located at path into a container
        std::optional<std::string> read(const std::string& path, lib::core::AudioContainer& container);

        // Writes data from a container into a file located at path, with requested encoding variables
        std::optional<std::string> write(const std::string& path, lib::core::AudioContainer& container,
                                        const lib::io::wav::encodingFormat& encodingFormat);

}


