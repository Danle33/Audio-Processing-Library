cmake_minimum_required(VERSION 4.0)
project(AudioLib)

set(CMAKE_CXX_STANDARD 17)

add_executable(AudioLib main.cpp
        include/lib/core/AudioChannel.hpp
        include/lib/core/AudioContainer.hpp
        include/lib/io/AudioIO.hpp
        src/AudioIO.cpp
        include/lib/dsp/AudioDSP.hpp
        src/AudioWav.cpp
        src/AudioDSP.cpp
        include/lib/io/AudioWAV.hpp
        include/lib/dsp/AudioEQ.hpp
        src/AudioEQ.cpp)
