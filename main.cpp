

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/io/AudioIO.hpp"

int main() {
    lib::core::AudioContainer myContainer;

    lib::io::read("input.wav", myContainer);

    // DSP processing...

    lib::io::write("output.wav", myContainer,
                    lib::io::wav::encodingFormat {
                        .sampleRateHz = 44100,
                        .bitDepth = 24,
                        .numChannels = 2
                    });

    return 0;
}
