# Digital Audio/Signal Processing Library

Built from scratch without external libraries, this library allows users to manipulate audio files via code.
Project focuses on deep low-level understanding of how audio gets decoded, processed and encoded at the end, with user-friendly APIs.

All DSP API methods are placed inside include/lib/dsp subdirectory. 
Users can perform general-type processing via methods from lib::dsp namespace.
More specialized processors extend the lib::dsp namespace and are placed in the same subdirectory.

Currently supported processors are:  
**Equalizer** which performs **filtering** (low-cut / high/cut) as well as **bell curved** type boosting and cutting.  
**Compressor** that can act as a limiter with ratio set to 100.  
**Resampler** which downsamples or upsamples with three different interpolation algorithms: **linear** (simple, low quality), **cubic** (average quality) and **sinc** (highest quality).  
Other general DSP options are **volume/gain changing**, **converting from stereo to mono**, **audio trimming** etc.. Others can be found in include/lib/GeneralDSP.hpp in namespace lib::dsp.

It is also possible to generate simple sound waves (currently only pure sine wave) at a given frequency, with custom amplitude and phase. All generator-related calls can be found in include/lib/generator/Generator.hpp in namespace lib::generator.

I/O methods are located in include/lib/io.
Project only supports .wav files for now.

## Building

Requirement: C++17

// TODO
