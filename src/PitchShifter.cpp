#include "../include/lib/dsp/PitchShifter.hpp"

#include <cmath>
#include <complex>
#include <algorithm>
#include <vector>

#include "../include/lib/core/Constants.hpp"
#include "../include/lib/dsp/Resampler.hpp"

namespace lib::dsp {

   std::optional<std::string> PitchShifter::process(lib::core::AudioContainer& container,
                                                     const double semitones,
                                                     const double formantShift) {

        if (container.isEmpty()) return "Error! Container is empty";
        if (semitones < -24.0 || semitones > 24.0) return "Semitones should be in range [-24, 24].";
        if (formantShift < -24.0 || formantShift > 24.0) return "Formant shift should be in range [-24, 24].";

        return std::nullopt;
    }

}
