#include "../include/lib/io/AudioIO.hpp"

#include <cstring>
#include <fstream>

#include "../include/lib/dsp/AudioDSP.hpp"


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
                                        const wav::encodingFormat& encodingFormat) {
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

std::optional<std::string> lib::io::wav::decode(const std::vector<uint8_t>& buffer, lib::core::AudioContainer& container) {
    if (buffer.size() < 12) return "Error! File size is less than 12 bytes.";

    char riff[4];
    std::memcpy(riff, buffer.data(), 4);
    if (memcmp(riff, "RIFF", 4) != 0) return "Error! RIFF chunk is missing.";

    uint32_t totalFileSize;
    std::memcpy(&totalFileSize, buffer.data() + 4, 4);
    if (static_cast<int32_t>(totalFileSize) + 8 <= 0) return "Error! File size field holds a negative value.";
    totalFileSize += 8;

    const uint32_t maxBoundary = std::min<size_t>(buffer.size(), totalFileSize);

    char wave[4];
    std::memcpy(wave, buffer.data() + 8, 4);
    if (memcmp(wave, "WAVE", 4) != 0) return "Error! WAVE chunk is missing.";

    uint32_t cursor = 12;
    bool fmtFound = false;
    bool dataFound = false;

    FmtPayload fmtPayload{};
    uint32_t dataPayloadCursor{};
    uint32_t dataPayloadSize{};

    while (cursor < totalFileSize) {
        if (cursor + 8 > maxBoundary) return "Error! Cursor exceeded file size.";

        char chunkID[4];
        std::memcpy(chunkID, buffer.data() + cursor, 4);

        uint32_t chunkSize;
        std::memcpy(&chunkSize, buffer.data() + cursor + 4, 4);

        if (std::memcmp(chunkID, "fmt ", 4) == 0) {
            if (fmtFound) return "Error! Multiple fmt chunks found.";
            fmtFound = true;

            uint32_t fmtPayloadCursor = cursor + 8;

            if (chunkSize < 16) return "Error! fmt chunk size is less than 16 bytes.";

            if (fmtPayloadCursor + 16 > maxBoundary) return "Error! Cursor exceeded file size.";
            std::memcpy(&fmtPayload, buffer.data() + fmtPayloadCursor, 16);

            if (fmtPayload.audioFormat == 65534 && chunkSize >= 40) {
                if (fmtPayloadCursor + 26 > maxBoundary) return "Error! Cursor exceeded file size.";
                std::memcpy(&fmtPayload.audioFormat, buffer.data() + fmtPayloadCursor + 24, 2);
            }
        }
        else if (std::memcmp(chunkID, "data", 4) == 0) {
            if (dataFound) return "Error! Multiple data chunks found.";
            dataFound = true;

            dataPayloadCursor = cursor + 8;
            dataPayloadSize = chunkSize;

            if (dataPayloadCursor + dataPayloadSize > maxBoundary) return "Error! Cursor exceeded file size.";
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

    if (!fmtFound || !dataFound) return "Error! File must contain both fmt and data chunks.";

    return populateChannels(buffer, fmtPayload, dataPayloadCursor, dataPayloadSize, container);
}

std::optional<std::string> lib::io::wav::populateChannels(const std::vector<uint8_t>& buffer, const FmtPayload& fmtPayload, uint32_t dataPayloadCursor, const uint32_t dataPayloadSize,
            lib::core::AudioContainer& container) {

    if (fmtPayload.numChannels != 1 && fmtPayload.numChannels != 2) return "Error! Decoded number of channels is invalid.";

    container.channels_.resize(fmtPayload.numChannels);

    const uint16_t bytesPerSample = fmtPayload.bitsPerSample / 8;
    const uint16_t frameSizeBytes = fmtPayload.numChannels * bytesPerSample;

    if (frameSizeBytes == 0) return "Error! Frame size is 0 bytes.";

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
    return std::nullopt;
}

std::optional<std::string> lib::io::wav::encode(std::vector<uint8_t>& buffer, lib::core::AudioContainer& container,
                            const encodingFormat& encodingFormat) {

    // validation
    if (encodingFormat.sampleRateHz != container.sampleRate_) {
        // TODO: resample(newSampleRate, container);
        return "Requested sample rate must be the same as input one.";
    }
    if (encodingFormat.bitDepth != 16 && encodingFormat.bitDepth != 24 && encodingFormat.bitDepth != 32)
        return "Invalid bit depth. Available: 16, 24, 32.";

    if (encodingFormat.numChannels == 2 && container.numChannels_ == 1) return "Cannot encode mono buffer into a stereo one";

    if (encodingFormat.numChannels == 1 && container.numChannels_ == 2) {
        auto status = lib::dsp::stereoToMono(container);
        if (status.has_value()) return status.value();
    }

    if (encodingFormat.numChannels != 1 && encodingFormat.numChannels != 2) return "Invalid number of channels.";

    uint8_t* bufferPtr = buffer.data();
    buffer.reserve(44);

    std::memcpy(bufferPtr, "RIFF", 4);
    bufferPtr += 4;

    const uint32_t frameCount = container.channels_[0].data_.size();
    const uint16_t blockAlign = encodingFormat.numChannels * (encodingFormat.bitDepth / 8);
    const uint32_t subchunk2Size = frameCount * blockAlign;

    const uint32_t chunkSize = subchunk2Size + 36;
    std::memcpy(bufferPtr, &chunkSize, 4);
    bufferPtr += 4;

    std::memcpy(bufferPtr, "WAVE", 4);
    bufferPtr += 4;

    std::memcpy(bufferPtr, "fmt ", 4);
    bufferPtr += 4;

    const uint32_t subchunk1Size = 16;
    std::memcpy(bufferPtr, &subchunk1Size, 4);
    bufferPtr += 4;

    const uint16_t audioFormat = 1;
    std::memcpy(bufferPtr, &audioFormat, 2);
    bufferPtr += 2;

    std::memcpy(bufferPtr, &encodingFormat.numChannels, 2);
    bufferPtr += 2;

    std::memcpy(bufferPtr, &encodingFormat.sampleRateHz, 4);
    bufferPtr += 4;

    std::memcpy(bufferPtr, &blockAlign, 2);
    bufferPtr += 2;

    std::memcpy(bufferPtr, &encodingFormat.bitDepth, 2);
    bufferPtr += 2;

    std::memcpy(bufferPtr, "data", 4);
    bufferPtr += 4;

    std::memcpy(bufferPtr, &subchunk2Size, 4);
    bufferPtr += 4;

    if (bufferPtr - buffer.data() != 44) return "Error! Buffer pointer offset is not 44 bytes after header encoding.";



    return std::nullopt;
}

