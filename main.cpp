

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/Compressor.hpp"
#include "include/lib/dsp/GeneralDSP.hpp"
#include "include/lib/dsp/EQ.hpp"
#include "include/lib/io/IO.hpp"
#include "include/lib/generator/Generator.hpp"

int main() {
    lib::core::AudioContainer myContainer;
    lib::io::read("Assets/Riff.wav", myContainer);

    lib::dsp::Equalizer eq;
    eq.highCut(myContainer, 3000, 24, 1);



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
