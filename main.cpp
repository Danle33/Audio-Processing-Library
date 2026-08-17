

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/Compressor.hpp"
#include "include/lib/dsp/Delay.hpp"
#include "include/lib/dsp/GeneralDSP.hpp"
#include "include/lib/dsp/EQ.hpp"
#include "include/lib/dsp/PitchShifter.hpp"
#include "include/lib/dsp/Resampler.hpp"
#include "include/lib/io/IO.hpp"
#include "include/lib/generator/Generator.hpp"

extern void exampleUsage();

int main() {

    lib::core::AudioContainer myContainer;
    lib::io::read("Assets/riff.wav", myContainer);

    lib::dsp::trim(myContainer, 0.4, 0.5);
    lib::dsp::stereoToMono(myContainer);

    lib::dsp::Delay delay;
    delay.process(myContainer, 500, 0.5, 0.5, true);

    lib::io::write("Assets/riffProcessed.wav", myContainer, lib::io::wav::encodingFormat {myContainer.sampleRate_, 16, 2});

    return 0;
}
