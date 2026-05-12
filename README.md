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

- ✅ Modular Architecture (Core, AI, Analytics, Cloud, IO, UI)
- ✅ Advanced AI: NeRF, Video Diffusion, Style Transfer, Neural CA
- ✅ Real-time Analytics: FFT, PCA, Granger Causality, Connectivity
- ✅ Cloud Ecosystem: AWS S3/Lambda/Kinesis sync, Vault auth
- ✅ Multi-format Exporters: PNG, GIF, MIDI, BMP, SVG
- ✅ Hierarchical Brain Modeling with LTP and Neurotransmitters
- ✅ High-performance ASCII/ANSI Visualization Engine (2D/3D/Grid)

---

## 🏗️ Architecture

QuantaCerebra follows a modular, domain-driven architecture for maximum portability and extensibility.

```
QuantaCerebra/
├── src/
│   ├── 🧬 core/                  # Simulation engine, Atlas, Modeling
│   ├── 🤖 ai/                    # Neural CA, Diffusion, NeRF, LLM Hub
│   ├── 📊 analytics/             # FFT, PCA, Statistics, Connectivity
│   ├── ☁️ cloud/                  # S3, Lambda, Kinesis, Vault, DB
│   ├── 📁 io/                     # JSON/XML/YAML parsers, Exporters
│   ├── 🖥️ ui/                     # Interactive TTY, CLI, Themes
│   ├── 🎥 visualization/          # Renderers (2D/3D/Grid/Particles)
│   └── 📄 main.cpp               # Application Entry Point
├── tests/
│   ├── 🧪 unit/                  # Modular Unit Tests
│   ├── 🧪 bdd/                   # Feature-based Integration Tests
│   ├── 🧪 e2e/                   # Full Workflow Verification
│   └── 🧪 sdd/                   # Sorrel Driven Development Audits
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

#### Using CMake (Recommended)
```bash
mkdir build && cd build
cmake ..
make
```

#### Direct Compilation (Minimal)
```bash
g++ -std=c++17 src/main.cpp src/core/*.cpp src/io/*.cpp src/ui/*.cpp src/visualization/*.cpp src/ai/*.cpp src/analytics/*.cpp src/api/*.cpp src/cloud/*.cpp -Isrc -o QuantaCerebra
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

QuantaCerebra employs a comprehensive 4-tier testing architecture:

### 1. Unit Testing (`tests/unit/`)
Extensive coverage for all modules including:
- **Core**: Atlas logic, neurochemistry, simulation engine.
- **IO**: Multi-format parsers (JSON/XML/YAML/CSV), hardware simulation.
- **AI/Analytics**: FFT/PCA accuracy, Diffusion/NeRF raymarching bounds.
- **Security**: Buffer overflow and command injection mitigation.

### 2. BDD Testing (`tests/bdd/`)
Feature-based integration tests following **Given/When/Then** methodology:
- `atlas.feature`: Verified region and pathway definitions.
- `rendering.feature`: Cross-layout (Grid/3D/List) visual parity.
- `playback.feature`: Temporal physics (decay, smoothing) verification.

### 3. E2E Testing (`tests/e2e/`)
Full workflow verification including CLI arguments, interactive TTY sessions, and serial port streaming.

### 4. SDD Audits (`tests/sdd/`)
Sorrel Driven Development audits that verify system resilience, architectural constraints, and empirical fact validation.

---

## 🔮 Future Roadmap

### Phase 1: Deep Integration
- [ ] Real-time hardware EEG integration
- [ ] VR/AR Streaming support
- [ ] Distributed simulation mode (P2P Mesh)

### Phase 2: High-Fidelity Rendering
- [ ] GPU-accelerated raymarching for 3D views
- [ ] SVG-based interactive dashboards
- [ ] Advanced Haptic feedback drivers

### Phase 3: Autonomous AI
- [ ] Self-evolving neural architectures via CA
- [ ] Adaptive neurotransmitter balance through RL
- [ ] Natural language simulation steering

---

## 📋 API Reference

### Core Classes

#### `cerebra::RegionState`
```cpp
struct RegionState {
    std::string region;                  // Brain region identifier
    double intensity;                    // Activity intensity [0.0-1.0]
    std::vector<NeurotransmitterFlow> flows; // Chemical signal data
    std::map<std::string, double> metrics;   // Custom region metrics
    std::vector<RegionState> subregions;     // Hierarchical child regions
};
```

#### `cerebra::BrainFrame`
```cpp
struct BrainFrame {
    int64_t timestamp_ms;                // Frame timestamp
    std::vector<RegionState> regions;    // Region activity data
};
```

### Key Functions (Data Hub)

#### `cerebra::parse_json_frames(std::string_view json)`
- **Purpose**: Parse JSON input into structured brain frame data.
- **Returns**: `std::vector<cerebra::BrainFrame>`

#### `cerebra::generateFrames(const std::vector<BrainFrame>& input, const AppConfig& config)`
- **Purpose**: Convert brain frames to ASCII/ANSI visualization based on configuration.
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
