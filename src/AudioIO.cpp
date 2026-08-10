#include "../include/lib/io/AudioIO.hpp"

#include <cstring>
#include <fstream>


bool lib::io::read(const std::string& path, lib::core::AudioContainer& container) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        return false;
    }

    const std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(fileSize);

    // 4. Read bytes directly into buffer vector memory
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        return false;
    }

    file.close();
    return wav::decodeWav(buffer, container);
}

bool lib::io::wav::decodeWav(std::vector<uint8_t>& buffer, lib::core::AudioContainer& container) {
#pragma pack(push, 1)
    struct WavHeader {
        char riff[4];            // "RIFF"
        uint32_t chunkSize;      // total file size - 8 bytes
        char wave[4];            // "WAVE"
        char fmt[4];             // "fmt "
        uint32_t fmtSize;        // 16
        uint16_t audioFormat;    // 1 = PCM, 3 = Float
        uint16_t numChannels;    // 1 or 2
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;  // 16, 24, 32...
        char data[4];            // "data"
        uint32_t dataSize;
    };
#pragma pack(pop)

    if (buffer.size() < sizeof(WavHeader)) return false;

    WavHeader header{};
    std::memcpy(&header, buffer.data(), sizeof(WavHeader));

    if (buffer.size() < sizeof(WavHeader) + header.dataSize) return false;

    const uint8_t* pPayload = buffer.data() + sizeof(WavHeader);

    if (header.bitsPerSample < 8) return false;
    const size_t totalSamples = header.dataSize / (header.bitsPerSample / 8);

    if (header.numChannels == 1) {
        std::vector<float> normalizedBuffer(totalSamples);

        const auto normalizingFactor = static_cast<float>(1UL << (header.bitsPerSample - 1));

        if (header.bitsPerSample == 8 && header.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const uint8_t*>(pPayload);

            for (size_t i = 0; i < totalSamples; ++i) {
                normalizedBuffer[i] = (pSamples[i] - normalizingFactor) / normalizingFactor;
            }
        }

        else if (header.bitsPerSample == 16 && header.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const int16_t*>(pPayload);

            for (size_t i = 0; i < totalSamples; ++i) {
                normalizedBuffer[i] = pSamples[i] / normalizingFactor;
            }
        }

        else if (header.bitsPerSample == 24 && header.audioFormat == 1) {
            for (size_t i = 0; i < totalSamples; ++i) {
                int32_t rawSample = (pPayload[3*i]) | (pPayload[3*i + 1] << 8) | (pPayload[3*i + 2] << 16);

                // sign extending to 32 bit
                if (rawSample & 0x00800000)
                    rawSample |= 0xFF000000;

                normalizedBuffer[i] = (rawSample)  / normalizingFactor;
            }
        }

        else if (header.bitsPerSample == 32 && header.audioFormat == 3) {
            const auto pSamples = reinterpret_cast<const float*>(pPayload);

            for (size_t i = 0; i < totalSamples; ++i) {
                normalizedBuffer[i] = pSamples[i];
            }
        }


        container.numChannels_ = header.numChannels;
        container.channels_.resize(header.numChannels);
        container.channels_[0].data_ = normalizedBuffer;
        container.sampleRate_ = header.sampleRate;
    }
    else if (header.numChannels == 2) {
        size_t totalFrames = totalSamples / header.numChannels;
        std::vector<float> normalizedBufferLeft(totalFrames);
        std::vector<float> normalizedBufferRight(totalFrames);

        const auto normalizingFactor = static_cast<float>(1UL << (header.bitsPerSample - 1));

        if (header.bitsPerSample == 8 && header.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const uint8_t*>(pPayload);

            for (size_t i = 0; i < totalFrames; ++i) {
                normalizedBufferLeft[i] = (pSamples[2*i] - normalizingFactor)  / normalizingFactor;
                normalizedBufferRight[i] = (pSamples[2*i + 1] - normalizingFactor)  / normalizingFactor;
            }
        }

        else if (header.bitsPerSample == 16 && header.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const int16_t*>(pPayload);

            for (size_t i = 0; i < totalFrames; ++i) {
                normalizedBufferLeft[i] = (pSamples[2*i])  / normalizingFactor;
                normalizedBufferRight[i] = (pSamples[2*i + 1])  / normalizingFactor;
            }
        }

        else if (header.bitsPerSample == 24 && header.audioFormat == 1) {

            for (size_t i = 0; i < totalFrames; ++i) {
                int32_t rawSampleLeft = (pPayload[6*i]) | (pPayload[6*i + 1] << 8) | (pPayload[6*i + 2] << 16);
                int32_t rawSampleRight = (pPayload[6*i + 3]) | (pPayload[6*i + 4] << 8) | (pPayload[6*i + 5] << 16);

                // sign extending to 32 bit
                if (rawSampleLeft & 0x00800000)
                    rawSampleLeft |= 0xFF000000;

                if (rawSampleRight & 0x00800000)
                    rawSampleRight |= 0xFF000000;

                normalizedBufferLeft[i] = (rawSampleLeft)  / normalizingFactor;
                normalizedBufferRight[i] = (rawSampleRight)  / normalizingFactor;
            }
        }

        else if (header.bitsPerSample == 32 && header.audioFormat == 3) {
            const auto pSamples = reinterpret_cast<const float*>(pPayload);

            for (size_t i = 0; i < totalFrames; ++i) {
                normalizedBufferLeft[i] = (pSamples[2*i]);
                normalizedBufferRight[i] = (pSamples[2*i + 1]);
            }

        }

        container.numChannels_ = header.numChannels;
        container.channels_.resize(header.numChannels);
        container.channels_[0].data_ = normalizedBufferLeft;
        container.channels_[1].data_ = normalizedBufferRight;
        container.sampleRate_ = header.sampleRate;
    }

    return true;
}

bool lib::io::wav::decodeWav(std::vector<char>& buffer, lib::core::AudioContainer& container) {
    char riff[4];
    std::memcpy(riff, buffer.data(), 4);
    if (strcmp(riff, "RIFF") != 0) return false;

    uint32_t totalFileSize;
    std::memcpy(&totalFileSize, buffer.data() + 4, 4);
    if (static_cast<int32_t>(totalFileSize) - 8 <= 0) return false;
    totalFileSize -= 8;

    char wave[4];
    std::memcpy(wave, buffer.data() + 8, 4);
    if (strcmp(wave, "WAVE") != 0) return false;

    uint32_t cursor = 12;
    bool fmtFound = false;
    bool dataFound = false;

    FmtPayload fmtPayload{};
    uint32_t dataPayloadCursor{};
    uint32_t dataPayloadSize{};

    while (cursor < totalFileSize) {
        char chunkID[4];
        std::memcpy(chunkID, buffer.data() + cursor, 4);

        uint32_t chunkSize;
        std::memcpy(&chunkSize, buffer.data() + cursor + 4, 4);

        if (strcmp(chunkID, "fmt ") == 0) {
            if (fmtFound) return false;
            fmtFound = true;

            uint32_t fmtPayloadCursor = cursor + 8;

            std::memcpy(&fmtPayload, buffer.data() + fmtPayloadCursor, 16);

            if (fmtPayload.audioFormat == 65534) {
                std::memcpy(&fmtPayload.audioFormat, buffer.data() + fmtPayloadCursor + 24, 2);
            }
        }
        else if (strcmp(chunkID, "data") == 0) {
            if (dataFound) return false;
            dataFound = true;

            dataPayloadCursor = cursor + 8;
            dataPayloadSize = chunkSize;

            if (dataPayloadCursor + dataPayloadSize > totalFileSize) return false;
        }

        if (fmtFound && dataFound) {
            break;
        }

        cursor += (chunkSize + (chunkSize % 2));
    }

    makeChannels(fmtPayload, dataPayloadCursor, dataPayloadSize, container);
    return true;
}

void lib::io::wav::makeChannels(const FmtPayload& fmtPayload, uint32_t dataPayloadCursor, const uint32_t dataPayloadSize,
            lib::core::AudioContainer& container) {

    container.channels_.resize(fmtPayload.numChannels);


}

