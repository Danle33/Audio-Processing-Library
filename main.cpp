

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/AudioCompressor.hpp"
#include "include/lib/dsp/AudioDSP.hpp"
#include "include/lib/dsp/AudioEQ.hpp"
#include "include/lib/io/AudioIO.hpp"
#include "include/lib/signalGeneration/AudioGenerator.hpp"

int main() {
    lib::core::AudioContainer myContainer;

    // DSP calls...
    lib::audioGenerator::generateSineWave(myContainer, 440, 1, 0, 3, 48000);

    double peak;
    lib::dsp::measurePeak(myContainer, &peak);
    std::cout << peak << std::endl;

    if (const auto status = lib::io::write("Assets/SineWave440Hz.wav", myContainer,
                    lib::io::wav::encodingFormat {
                        .sampleRateHz = 48000,
                        .bitDepth = 16,
                        .numChannels = 1
                    })) {
        std::cout << *status << '\n';
        return 0;
    }

    return 0;
}
