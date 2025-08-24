# 🧠 QuantaCerebra

[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()

**QuantaCerebra** is a high-performance C++17 console application designed to simulate and visualize brain activity through lightweight ASCII-based video generation. Built without external dependencies, it provides real-time frame-by-frame visualizations of neural activity patterns from structured JSON input data, while also serving as a foundation for experimenting with advanced graphics modeling, such as video diffusion, on limited resources.

---

## 🎯 Project Overview

### Core Objectives

- **Dynamic Brain Activity Modeling**: Simulate neural activity as temporal sequences of region-based intensity readings
- **ASCII Visualization Engine**: Generate real-time frame-by-frame animations reflecting regional intensity changes
- **Zero-Dependency Architecture**: Maintain strict portability using only standard C++17 libraries
- **Extensible Foundation**: Provide a robust base for future integrations of advanced graphics modeling, such as video diffusion, to enhance visualizations and explore novel representations of brain activity on resource-constrained hardware.

### Key Features

- ✅ Manual JSON parsing without external libraries
- ✅ Real-time ASCII animation rendering
- ✅ Configurable intensity mapping system
- ✅ Comprehensive unit and BDD testing suite
- ✅ Cross-platform compatibility

---

## 🏗️ Architecture

The project is organized into `src`, `tests`, and `docs` directories, separating core logic, testing, and documentation.

```
QuantaCerebra/
├── src/
│   ├── 📄 main.cpp               # Application entry point
│   ├── 🔧 config.h/.cpp          # Configuration management
│   ├── 🔧 json_logic.h/.cpp     # Custom JSON parser
│   └── 🎥 video_logic.h/.cpp     # Frame generation and visualization
├── tests/
│   ├── 🧪 test_unit.cpp          # Unit test suite
│   └── 🧪 test_bdd.cpp           # BDD integration tests
├── docs/
│   ├── 📄 enhancements.md       # Notes on future enhancements
│   └── 📄 plan.md               # Development plan
├── 📊 sample_input.json          # Sample brain activity data for testing
├── 📋 expected_output.txt        # Expected visualization output for tests
├── 📄 BUILD_INSTRUCTIONS.md     # Detailed build instructions
├── 📄 simple_build.cpp          # A single-file build version for convenience
├── 📄 config.ini                # Configuration file
├── 📄 build.bat                 # Windows build script
└── 📖 README.md                 # Project documentation
```

---

## 📊 Data Format Specification

### Input JSON Schema

The application processes brain activity data in the following JSON format:

```json
[
  {
    "brain_activity": [
      { "region": "prefrontal_cortex", "intensity": 0.8 },
      { "region": "amygdala", "intensity": 0.6 },
      { "region": "occipital_lobe", "intensity": 0.9 }
    ],
    "timestamp_ms": 0
  }
]
```

#### Schema Components

| Field | Type | Description | Range |
|-------|------|-------------|-------|
| `timestamp_ms` | Integer | Frame timestamp in milliseconds | ≥ 0 |
| `brain_activity` | Array | Collection of brain region data | 1+ regions |
| `region` | String | Brain region identifier | Any valid string |
| `intensity` | Float | Neural activity intensity | 0.0 - 1.0 |

---

## 🎨 Visualization System

### Intensity Mapping

The visualization engine maps neural activity intensities to ASCII characters:

| Intensity Range | Symbol | Visual Representation |
|----------------|--------|----------------------|
| `< 0.2` | `' '` | Minimal activity (space) |
| `0.2 - 0.4` | `'.'` | Low activity (dot) |
| `0.4 - 0.6` | `':'` | Moderate activity (colon) |
| `0.6 - 0.75` | `'+'` | High activity (plus) |
| `0.75 - 0.9` | `'X'` | Very high activity (X) |
| `≥ 0.9` | `'@'` | Maximum activity (at symbol) |

### Output Format

```
[Time: 0 ms]
[prefrontal_cortex] XXXXXXXX
[amygdala] ++++++
[occipital_lobe] @@@@@@@@@@

[Time: 66 ms]
[prefrontal_cortex] +++++
[amygdala] ...
[occipital_lobe] ··
```

---

## 🚀 Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Standard C++ library

### Build Instructions

#### Compile Main Application
```bash
g++ src/main.cpp src/json_logic.cpp src/video_logic.cpp src/config.cpp -std=c++17 -o QuantaCerebra
```

#### Compile Test Suites
```bash
# Unit tests
g++ tests/test_unit.cpp src/json_logic.cpp src/video_logic.cpp src/config.cpp -std=c++17 -o test_unit

# BDD integration tests
g++ tests/test_bdd.cpp src/json_logic.cpp src/video_logic.cpp src/config.cpp -std=c++17 -o test_bdd
```

### Execution

#### Run Application
```bash
./QuantaCerebra
```

#### Run Tests
```bash
./test_unit    # Execute unit tests
./test_bdd     # Execute BDD tests
```

---

## 🧪 Testing Framework

### Unit Testing (`test_unit.cpp`)

Validates core functionality:
- ✅ JSON parsing accuracy
- ✅ Intensity-to-character mapping
- ✅ Error handling for malformed input
- ✅ Data structure integrity

### BDD Testing (`test_bdd.cpp`)

Follows **Given/When/Then** methodology:
- **Given**: `sample_input.json` with test data
- **When**: Frames are generated through the pipeline
- **Then**: Output matches `expected_output.json`

---

## 🔮 Future Roadmap

### Phase 1: Enhanced Modeling
- [ ] Hierarchical brain region support (nested subregions)
- [ ] Temporal smoothing and decay models
- [ ] Real-time data stream processing
- [ ] Configuration file support

### Phase 2: Advanced Visualization
- [ ] Grid/matrix brain map layouts
- [ ] ANSI color-coded terminal output
- [ ] Export capabilities (PNG/JPEG without external libs)
- [ ] Interactive visualization controls

### Phase 3: AI Integration
- [ ] **Frame Interpolation**: RIFE/FILM-based frame blending
- [ ] **Dynamic Panning**: Ken Burns-style ASCII animations
- [ ] **GAN Integration**: StyleGAN/VQGAN for enhanced visuals
- [ ] **Neural Radiance Fields**: 3D cortex rendering (Instant-NGP)
- [ ] **Diffusion Models**: VideoCrafter2, PixArt-α, LCM integration

### Phase 4: Ecosystem Development
- [ ] LLM-driven input generation (llama.cpp integration)
- [ ] Natural language configuration templating
- [ ] Feedback loop systems for adaptive visualization
- [ ] Plugin architecture for custom renderers

---

## 📋 API Reference

### Core Classes

#### `BrainRegion`
```cpp
struct BrainRegion {
    std::string region_name;  // Brain region identifier
    double intensity;         // Activity intensity [0.0-1.0]
};
```

#### `BrainFrame`
```cpp
struct BrainFrame {
    int timestamp_ms;                    // Frame timestamp
    std::vector<BrainRegion> regions;    // Region activity data
};
```

### Key Functions

#### `parseBrainActivityJSON(const std::string& json)`
- **Purpose**: Parse JSON input into structured brain frame data
- **Parameters**: Raw JSON string
- **Returns**: `std::vector<BrainFrame>`

#### `generateFrames(const std::vector<BrainFrame>& input)`
- **Purpose**: Convert brain frames to ASCII visualization
- **Parameters**: Vector of brain frames
- **Returns**: `std::vector<std::string>` (rendered frames)

---

## 🤝 Contributing

We welcome contributions! Please follow these guidelines:

1. **Fork** the repository
2. **Create** a feature branch (`git checkout -b feature/amazing-feature`)
3. **Commit** your changes (`git commit -m 'Add amazing feature'`)
4. **Push** to the branch (`git push origin feature/amazing-feature`)
5. **Open** a Pull Request

### Code Standards
- Follow C++17 best practices
- Maintain zero external dependencies
- Include comprehensive tests
- Document all public APIs

---

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---

## 🏆 Acknowledgments

**Concept & Architecture**: Tamaro Green  
**Implementation**: AI-Assisted Development using modular C++ design principles

---

## 📞 Support

For questions, issues, or feature requests:
- 🐛 **Bug Reports**: Use GitHub Issues
- 💡 **Feature Requests**: Use GitHub Discussions
- 📧 **Contact**: [Your Contact Information]

---

<div align="center">

**QuantaCerebra** - *Visualizing the Mind, Frame by Frame*

[![GitHub stars](https://img.shields.io/github/stars/username/quantacerebra.svg?style=social&label=Star)](https://github.com/username/quantacerebra)
[![GitHub forks](https://img.shields.io/github/forks/username/quantacerebra.svg?style=social&label=Fork)](https://github.com/username/quantacerebra/fork)

</div>
