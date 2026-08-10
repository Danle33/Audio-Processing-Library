#pragma once
#include <string>

#include "../core/AudioContainer.hpp"

namespace lib::io {
        bool read(const std::string& path, lib::core::AudioContainer& container);
        bool write(const std::string& path, lib::core::AudioContainer& container);

}

namespace lib::io::wav {

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

        bool decodeWav(std::vector<char>& buffer, lib::core::AudioContainer& container);
        void makeChannels(const FmtPayload& fmtPayload, uint32_t dataPayloadCursor, uint32_t dataPayloadSize,
                        lib::core::AudioContainer& container);
}
