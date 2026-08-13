

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/AudioCompressor.hpp"
#include "include/lib/dsp/AudioDSP.hpp"
#include "include/lib/dsp/AudioEQ.hpp"
#include "include/lib/io/AudioIO.hpp"

int main() {
    lib::core::AudioContainer myContainer;

    if (const auto status = lib::io::read("Assets/Riff.wav", myContainer)) {
        std::cout << *status << '\n';
        return 0;
    }

    // DSP calls...
    lib::dsp::trim(myContainer, 0, 40);
    lib::dsp::volumeDecibels(myContainer, 1.0);

    double peak;
    lib::dsp::measurePeak(myContainer, &peak);
    std::cout << peak << '\n';

    double gainReduction;
    lib::dsp::compressor::compress(myContainer, -50, 4, 1, 100, 1, &gainReduction);
    std::cout << gainReduction << '\n';

    lib::dsp::volumeDecibels(myContainer, gainReduction);

    if (const auto status = lib::io::write("Assets/RiffProcessed.wav", myContainer,
                    lib::io::wav::encodingFormat {
                        .sampleRateHz = 44100,
                        .bitDepth = 16,
                        .numChannels = 2
                    })) {
        std::cout << *status << '\n';
        return 0;
    }

    return 0;
}
