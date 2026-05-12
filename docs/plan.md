# QuantaCerebra Detailed Development Plan

This document outlines the detailed development plan for QuantaCerebra, breaking down the strategic roadmap into actionable steps.

---

## Phase 1: Core Functionality Enhancement [COMPLETED]

This phase focuses on enhancing the core modeling and data processing capabilities of the application.

### 1.1 Hierarchical Brain Region Support [✅]

**Objective**: Implement support for nested subregions to create more detailed brain models.

-   [x] **Step 1: Update Data Structures**:
    -   Modified `cerebra::RegionState` to recursively support subregions.
-   [x] **Step 2: Enhance JSON Parser**:
    -   Updated `json_parser.cpp` to parse nested `subregions`.
-   [x] **Step 3: Refine Visualization Logic**:
    -   Modified `video_logic.cpp` to recursively process and render hierarchical regions with indentation.
-   [x] **Step 4: Update Sample Data**:
    -   Updated `data/builtin_atlas.json` to reflect hierarchical structures.

### 1.2 Configuration File Support [✅]

**Objective**: Allow users to configure simulation parameters via an external file.

-   [x] **Step 1: Design Configuration System**:
    -   Created `io/config.h` and `io/config.cpp` with `AppConfig` struct.
-   [x] **Step 2: Implement Configuration Loading**:
    -   Implemented JSON-based configuration loading in `io/config.cpp`.
-   [x] **Step 3: Integrate Configuration Parameters**:
    -   Integrated `enable_color`, `smoothing_window_size`, `layout_mode`, and advanced physics flags.

---

## Phase 2: Advanced Visualization [COMPLETED]

This phase aims to improve the visual output and user interaction.

### 2.1 ANSI Color-Coded Terminal Output [✅]

**Objective**: Add color to the terminal output for a richer visualization.

-   [x] **Step 1: Define ANSI Color Codes**:
    -   Implemented dynamic 256-color mapping in `video_logic.cpp`.
-   [x] **Step 2: Implement Color Mapping**:
    -   Created themes (Ocean, Forest, Dynamic) mapping intensity to color.
-   [x] **Step 3: Add Configuration Flag**:
    -   Integrated with `enable_color` parameter.

### 2.2 Grid/Matrix Brain Map Layouts [✅]

**Objective**: Implement a 2D grid layout for visualizing brain regions.

-   [x] **Step 1: Design Grid Layout**:
    -   Implemented coordinate-based mapping for brain regions.
-   [x] **Step 2: Update Rendering Logic**:
    -   Created `renderGrid` and `render3D` in `video_logic.cpp`.
-   [x] **Step 3: Add Layout Switching**:
    -   Switching via `layout_mode` in `AppConfig` or CLI.

---

## Phase 3: Future Development (High-Level)

This section outlines future work that will be detailed in subsequent planning sessions.

-   **Temporal Smoothing and Decay Models**: Introduce more realistic temporal dynamics.
-   **Real-time Data Stream Processing**: Adapt the application for real-time data sources.
-   **Export Capabilities**: Allow exporting visualizations to common formats like PNG/JPEG without external libraries.
-   **AI/ML Integration**: Explore advanced AI-driven features like frame interpolation and generative visuals.
