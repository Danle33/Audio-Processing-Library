cmake_minimum_required(VERSION 4.0)
project(AudioLib)

set(CMAKE_CXX_STANDARD 17)

add_executable(AudioLib main.cpp
        include/lib/core/AudioChannel.hpp
        include/lib/core/AudioContainer.hpp
        include/lib/io/AudioFileHandler.hpp
        src/AudioFileHandler.cpp
        include/lib/io/AudioDecoder.hpp
        src/AudioDecoder.cpp)
