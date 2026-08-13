

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
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
    lib::dsp::trim(myContainer, 50, 100);
    lib::dsp::eq::bell(myContainer, 3000, 5, 1);

    if (const auto status = lib::io::write("Assets/RiffProcessed.wav", myContainer,
                    lib::io::wav::encodingFormat {
                        .sampleRateHz = 44100,
                        .bitDepth = 16,
                        .numChannels = 1
                    })) {
        std::cout << *status << '\n';
        return 0;
    }

    return 0;
}
