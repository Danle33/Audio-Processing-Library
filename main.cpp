

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/Compressor.hpp"
#include "include/lib/dsp/GeneralDSP.hpp"
#include "include/lib/dsp/EQ.hpp"
#include "include/lib/dsp/PitchShifter.hpp"
#include "include/lib/dsp/Resampler.hpp"
#include "include/lib/io/IO.hpp"
#include "include/lib/generator/Generator.hpp"

int main() {

    lib::core::AudioContainer hz440;
    lib::core::AudioContainer hz480;

    lib::audioGenerator::generateSineWave(hz440, 440, 0.5, 0, 3, 44100);
    lib::audioGenerator::generateSineWave(hz480, 480, 0.5, 0, 3, 44100);

    lib::core::AudioContainer myContainer;
    auto vec = std::vector<lib::core::AudioContainer>{hz440, hz480};
    lib::audioGenerator::joinSineWaves(myContainer, vec);

    double peak;
    lib::dsp::measurePeak(myContainer, &peak);
    std::cout << peak << std::endl;


    if (const auto status = lib::io::write("Assets/joined.wav", myContainer,
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
