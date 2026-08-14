#include "../include/lib/io/IO.hpp"
#include "../include/lib/io/Wav.hpp"

#include <cstring>
#include <fstream>

#include "../include/lib/dsp/GeneralDSP.hpp"


std::optional<std::string> lib::io::read(const std::string& path, lib::core::AudioContainer& container) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return "Error while opening a file.";
    }

    const std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return "Error while parsing file data.";
    }

    file.close();
    return wav::decode(buffer, container);
}

std::optional<std::string> lib::io::write(const std::string& path, lib::core::AudioContainer& container,
                                        const lib::io::wav::encodingFormat& encodingFormat) {
    std::vector<uint8_t> buffer;
    auto status = wav::encode(buffer, container, encodingFormat);
    if (status.has_value()) {
        return status.value();
    }

    std::ofstream file(path, std::ios::binary);

    if (!file.is_open()) {
        return "Error while opening a file.";
    }

    const auto fileSize = static_cast<std::streamsize>(buffer.size());

    if (!file.write(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return "Error while writing into a file.";
    }

    file.close();
    return std::nullopt;
}


