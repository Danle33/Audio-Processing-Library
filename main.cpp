

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/Compressor.hpp"
#include "include/lib/dsp/GeneralDSP.hpp"
#include "include/lib/dsp/EQ.hpp"
#include "include/lib/dsp/PitchShifter.hpp"
#include "include/lib/dsp/Resampler.hpp"
#include "include/lib/io/IO.hpp"
#include "include/lib/generator/Generator.hpp"

extern void exampleUsage();

int main() {

    lib::core::AudioContainer c1;
    lib::io::read("Assets/riff.wav", c1);

    lib::core::AudioContainer c2;
    lib::audioGenerator::generateSineWave(c2, 440, 0.5, 180, 180, 48000);

    lib::core::AudioContainer c3;
    std::vector v = {c1, c2};
    lib::dsp::joinSignals(c3, v);

    lib::io::write("Assets/joined.wav", c3, lib::io::wav::encodingFormat {44100, 16, 2});

    return 0;
}
