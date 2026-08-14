# Digital Audio & Signal Processing Library

A lightweight, zero-dependency C++17 audio processing library built from scratch. 

This project focuses on a low-level understanding of audio decoding, digital signal processing (DSP), and encoding under the hood—exposing these capabilities through a clean, intuitive C++ API.

---

## Features

### Signal Processing (`lib::dsp`)
* **Equalizer:** Low-cut, high-cut, and bell-curve boosting/cutting filters.
* **Compressor:** Dynamic range compression with adjustable thresholds, attack/release, and ratios (acts as a limiter when ratio is set to 100:1).
* **Resampler:** Upsampling and downsampling with three interpolation algorithms:
  * `Linear` – Low complexity, fast.
  * `Cubic` – Balanced performance and quality.
  * `Sinc` – High-fidelity signal reconstruction.
* **General DSP (`GeneralDSP.hpp`):** Gain/volume adjustments, stereo-to-mono downmixing, audio trimming, and buffer operations.

### Signal Generation (`lib::generator`)
* Synthesizes audio waveforms (e.g., pure sine waves) with custom frequency, amplitude, and initial phase parameters.

### Audio I/O (`lib::io`)
* Parsing and encoding support for **WAV** files (`.wav`).

---

## Directory Structure

```text
include/
└── lib/
    ├── core/          # Core audio containers and data types
    ├── dsp/           # Processors (Equalizer, Compressor, Resampler)
    ├── generator/     # Signal synthesis engine
    └── io/            # Audio file decoders and encoders (WAV)
