

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/io/AudioIO.hpp"

int main() {
    lib::core::AudioContainer myContainer;

    if (const auto status = lib::io::read("input.wav", myContainer)) {
        std::cout << *status << '\n';
        return 0;
    }

    // DSP processing...

    if (const auto status = lib::io::write("output.wav", myContainer,
                    lib::io::wav::encodingFormat {
                        .sampleRateHz = 44100,
                        .bitDepth = 24,
                        .numChannels = 2
                    })) {
        std::cout << *status << '\n';
        return 0;
    }

    return 0;
}
