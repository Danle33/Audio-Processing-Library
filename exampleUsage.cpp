

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/dsp/Compressor.hpp"
#include "include/lib/dsp/GeneralDSP.hpp"
#include "include/lib/dsp/EQ.hpp"
#include "include/lib/dsp/PitchShifter.hpp"
#include "include/lib/dsp/Resampler.hpp"
#include "include/lib/io/IO.hpp"
#include "include/lib/generator/Generator.hpp"

void exampleUsage() {

    lib::core::AudioContainer myContainer;

    if (const auto status = lib::io::read("Assets/riff.wav", myContainer)) {
        std::cout << *status << '\n';
        exit(-1);
    }

    lib::dsp::volumeDecibels(myContainer, -0.5);

    double peakBefore;
    lib::dsp::measurePeak(myContainer, &peakBefore);
    std::cout << "Peak before compression: " << peakBefore << '\n';

    double gainReduction;
    lib::dsp::Compressor compressor(-20, 4, 1, 50, 1);
    compressor.process(myContainer, &gainReduction);
    std::cout << "Gain reduction: " << gainReduction << '\n';

    double peakAfter;
    lib::dsp::measurePeak(myContainer, &peakAfter);
    std::cout << "Peak after compression: " << peakAfter << '\n';

    lib::dsp::volumeDecibels(myContainer, peakBefore - peakAfter); // Makeup gain

    double peak;
    lib::dsp::measurePeak(myContainer, &peak);
    std::cout << "Peak after makeup gain: " << peak << '\n';

    lib::dsp::Equalizer eq;
    eq.highCut(myContainer, 8000, 24, 1);

    lib::dsp::stereoToMono(myContainer);

    if (const auto status = lib::io::write("Assets/riffProcessed.wav", myContainer,
                    lib::io::wav::encodingFormat {
                        .sampleRateHz = 44100,
                        .bitDepth = 16,
                        .numChannels = 1
                    })) {
        std::cout << *status << '\n';
        exit(-1);
                        }

}
