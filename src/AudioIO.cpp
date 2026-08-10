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

bool lib::io::wav::decodeWav(const std::vector<uint8_t>& buffer, lib::core::AudioContainer& container) {
    if (buffer.size() < 12) return false;

    char riff[4];
    std::memcpy(riff, buffer.data(), 4);
    if (memcmp(riff, "RIFF", 4) != 0) return false;

    uint32_t totalFileSize;
    std::memcpy(&totalFileSize, buffer.data() + 4, 4);
    if (static_cast<int32_t>(totalFileSize) - 8 <= 0) return false;
    totalFileSize += 8;

    const uint32_t maxBoundary = std::min<size_t>(buffer.size(), totalFileSize);

    char wave[4];
    std::memcpy(wave, buffer.data() + 8, 4);
    if (memcmp(wave, "WAVE", 4) != 0) return false;

    uint32_t cursor = 12;
    bool fmtFound = false;
    bool dataFound = false;

    FmtPayload fmtPayload{};
    uint32_t dataPayloadCursor{};
    uint32_t dataPayloadSize{};

    while (cursor < totalFileSize) {
        if (cursor + 8 > maxBoundary) return false;

        char chunkID[4];
        std::memcpy(chunkID, buffer.data() + cursor, 4);

        uint32_t chunkSize;
        std::memcpy(&chunkSize, buffer.data() + cursor + 4, 4);

        if (std::memcmp(chunkID, "fmt ", 4) == 0) {
            if (fmtFound) return false;
            fmtFound = true;

            uint32_t fmtPayloadCursor = cursor + 8;

            if (chunkSize < 16 || fmtPayloadCursor + 16 > maxBoundary) return false;
            std::memcpy(&fmtPayload, buffer.data() + fmtPayloadCursor, 16);

            if (fmtPayload.audioFormat == 65534 && chunkSize >= 40) {
                if (fmtPayloadCursor + 26 > maxBoundary) return false;
                std::memcpy(&fmtPayload.audioFormat, buffer.data() + fmtPayloadCursor + 24, 2);
            }
        }
        else if (std::memcmp(chunkID, "data", 4) == 0) {
            if (dataFound) return false;
            dataFound = true;

            dataPayloadCursor = cursor + 8;
            dataPayloadSize = chunkSize;

            if (dataPayloadCursor + dataPayloadSize > maxBoundary) return false;
        }

        if (fmtFound && dataFound) {
            break;
        }

        const uint64_t nextCursor = static_cast<uint64_t>(cursor) + 8 + chunkSize + (chunkSize % 2);
        if (nextCursor > maxBoundary) {
            break;
        }
        cursor = static_cast<uint32_t>(nextCursor);
    }

    if (!fmtFound || !dataFound) return false;

    return populateChannels(buffer, fmtPayload, dataPayloadCursor, dataPayloadSize, container);
}

bool lib::io::wav::populateChannels(const std::vector<uint8_t>& buffer, const FmtPayload& fmtPayload, uint32_t dataPayloadCursor, const uint32_t dataPayloadSize,
            lib::core::AudioContainer& container) {

    if (fmtPayload.numChannels != 1 && fmtPayload.numChannels != 2) return false;

    container.channels_.resize(fmtPayload.numChannels);

    const uint16_t bytesPerSample = fmtPayload.bitsPerSample / 8;
    const uint16_t frameSizeBytes = fmtPayload.numChannels * bytesPerSample;

    if (frameSizeBytes == 0) return false;

    const uint32_t frameCount = dataPayloadSize / frameSizeBytes;

    container.numChannels_ = fmtPayload.numChannels;
    container.sampleRate_ = fmtPayload.sampleRate;

    // MONO
    if (fmtPayload.numChannels == 1) {
        container.channels_[0].data_.resize(frameCount);

        const auto normalizingFactor = static_cast<float>(1UL << (fmtPayload.bitsPerSample - 1));

        if (fmtPayload.bitsPerSample == 8 && fmtPayload.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const uint8_t*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[i] - normalizingFactor) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 16 && fmtPayload.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const int16_t*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[i]) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 24 && fmtPayload.audioFormat == 1) {
            const auto bufferData = buffer.data() + dataPayloadCursor;
            for (size_t i = 0; i < frameCount; ++i) {
                int32_t rawSample = (bufferData[3*i]) | (bufferData[3*i + 1] << 8) | (bufferData[3*i + 2] << 16);

                // sign extending to 32 bit
                if (rawSample & 0x00800000)
                    rawSample |= 0xFF000000;

                container.channels_[0].data_[i] = (rawSample) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 32 && fmtPayload.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const int32_t*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[i]) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 32 && fmtPayload.audioFormat == 3) {
            const auto pSamples = reinterpret_cast<const float*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[i]);
            }
        }

    }
    // STEREO
    else if (fmtPayload.numChannels == 2) {
        container.channels_[0].data_.resize(frameCount);
        container.channels_[1].data_.resize(frameCount);

        const auto normalizingFactor = static_cast<float>(1UL << (fmtPayload.bitsPerSample - 1));

        if (fmtPayload.bitsPerSample == 8 && fmtPayload.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const uint8_t*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[2*i] - normalizingFactor) / normalizingFactor;
                container.channels_[1].data_[i] = (pSamples[2*i + 1] - normalizingFactor) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 16 && fmtPayload.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const int16_t*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[2*i]) / normalizingFactor;
                container.channels_[1].data_[i] = (pSamples[2*i + 1]) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 24 && fmtPayload.audioFormat == 1) {
            const auto bufferData = buffer.data() + dataPayloadCursor;
            for (size_t i = 0; i < frameCount; ++i) {
                int32_t rawSampleLeft = (bufferData[6*i]) | (bufferData[6*i + 1] << 8) | (bufferData[6*i + 2] << 16);
                int32_t rawSampleRight = (bufferData[6*i + 3]) | (bufferData[6*i + 4] << 8) | (bufferData[6*i + 5] << 16);

                // sign extending to 32 bit
                if (rawSampleLeft & 0x00800000)
                    rawSampleLeft |= 0xFF000000;
                if (rawSampleRight & 0x00800000)
                    rawSampleRight |= 0xFF000000;

                container.channels_[0].data_[i] = (rawSampleLeft) / normalizingFactor;
                container.channels_[1].data_[i] = (rawSampleRight) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 32 && fmtPayload.audioFormat == 1) {
            const auto pSamples = reinterpret_cast<const int32_t*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[2*i]) / normalizingFactor;
                container.channels_[1].data_[i] = (pSamples[2*i + 1]) / normalizingFactor;
            }
        }

        else if (fmtPayload.bitsPerSample == 32 && fmtPayload.audioFormat == 3) {
            const auto pSamples = reinterpret_cast<const float*>(buffer.data() + dataPayloadCursor);

            for (size_t i = 0; i < frameCount; ++i) {
                container.channels_[0].data_[i] = (pSamples[2*i]);
                container.channels_[1].data_[i] = (pSamples[2*i + 1]);
            }
        }
    }
    return true;
}

