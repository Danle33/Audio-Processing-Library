# Digital Audio & Signal Processing Library

A lightweight, zero-dependency C++17 audio processing library built from scratch.

This project focuses on a low-level understanding of audio decoding, digital signal processing (DSP), and encoding under the hood—exposing these capabilities through a clean, intuitive C++ API.

---

## Features

### Signal Processing (`lib::dsp`)
* **Equalizer:** Low-cut, high-cut, and bell-curve boosting/cutting filters.
* **Compressor:** Dynamic range compression with adjustable thresholds, attack/release, lookahead time, and ratios (acts as a limiter when ratio is set to 100:1).
* **Resampler:** Upsampling and downsampling with three interpolation algorithms:
  * `Linear` – Low complexity, fast.
  * `Cubic` – Balanced performance and quality.
  * `Sinc` – High-fidelity signal reconstruction.
* **General DSP (`GeneralDSP.hpp`):** Gain/volume adjustments, stereo-to-mono downmixing, audio trimming, and peak measuring.

### Signal Generation (`lib::generator`)
* Synthesizes audio waveforms (e.g., pure sine waves) with custom frequency, amplitude, and initial phase parameters.

### Audio I/O (`lib::io`)
* Parsing and encoding support for **WAV** files (`.wav`).

*A complete code example can be found in `main.cpp`.*

---

## Building

### Prerequisites
* **C++ Compiler:** C++17 support required (GCC 9+, Clang 10+, MSVC 2019+)
* **Build System:** CMake 3.15 or higher

### Build Steps

**1. Clone the repository:**
```bash
git clone https://github.com/Danle33/Audio-Processing-Library.git
cd Audio-Processing-Library

```
**2. Configure the build directory:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release

```

**3. Compile the library:**
```bash
cmake --build build --config Release

```

### Running the Application

Once built, execute the compiled binary directly from the build directory:

* **Linux / macOS:**
```bash
./build/your_app_name

```

* **Windows (Command Prompt / PowerShell):**
```cmd
.\build\Release\your_app_name.exe

```

> **Note:** Important CMake Details to Keep in Mind

**Target Name vs. Project Name:** Ensure `your_app_name` matches the exact target inside your `CMakeLists.txt`:
```cmake
add_executable(audio_dsp_demo main.cpp)

```
  
