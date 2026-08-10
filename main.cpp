

#include <iostream>

#include "include/lib/core/AudioContainer.hpp"
#include "include/lib/io/AudioIO.hpp"

int main() {
    lib::core::AudioContainer myContainer;

    if (!lib::io::read("input.wav", myContainer)) {
        std::cerr << "Failed to read input file.\n";
        return 0;
    }

    // DSP processing...

    if (!lib::io::write("output.wav", myContainer)) {
        std::cerr << "Failed to write output file.\n";
        return 0;
    }

    return 0;
}
