cmake_minimum_required(VERSION 4.0)
project(AudioLib)

set(CMAKE_CXX_STANDARD 17)

add_executable(AudioLib main.cpp
        include/lib/core/AudioChannel.hpp
        include/lib/core/AudioContainer.hpp
        include/lib/io/IO.hpp
        src/IO.cpp
        include/lib/dsp/GeneralDSP.hpp
        src/Wav.cpp
        src/GeneralDSP.cpp
        include/lib/io/Wav.hpp
        include/lib/dsp/EQ.hpp
        src/EQ.cpp
        include/lib/dsp/Compressor.hpp
        src/Compressor.cpp
        include/lib/generator/Generator.hpp
        src/Generator.cpp
        include/lib/core/Constants.hpp
        include/lib/dsp/Resampler.hpp
        src/Resampler.cpp)
