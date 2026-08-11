#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "../core/AudioContainer.hpp"

namespace lib::io::wav {

    // Decoding
#pragma pack(push, 1)
    struct FmtPayload {
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
    };
#pragma pack(pop)

    std::optional<std::string> decode(const std::vector<uint8_t>& buffer, lib::core::AudioContainer& container);
    std::optional<std::string> populateChannels(const std::vector<uint8_t>& buffer, const FmtPayload& fmtPayload, uint32_t dataPayloadCursor,
                                            uint32_t dataPayloadSize,
                                            lib::core::AudioContainer& container);

    // Encoding
#pragma pack(push, 1)
    struct encodingFormat {
        uint32_t sampleRateHz;
        uint16_t bitDepth;
        uint16_t numChannels;
    };
#pragma pack(pop)

    std::optional<std::string> encode(std::vector<uint8_t>& buffer, lib::core::AudioContainer& container,
            const encodingFormat& encodingFormat);
}
