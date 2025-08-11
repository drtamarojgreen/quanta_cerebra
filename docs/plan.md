# QuantaCerebra Detailed Development Plan

This document outlines the detailed development plan for QuantaCerebra, breaking down the strategic roadmap into actionable steps.

---

## Phase 1: Core Functionality Enhancement

This phase focuses on enhancing the core modeling and data processing capabilities of the application.

### 1.1 Hierarchical Brain Region Support

**Objective**: Implement support for nested subregions to create more detailed brain models.

-   **Step 1: Update Data Structures**:
    -   Modify the `BrainRegion` struct in `video_logic.h` to recursively support subregions.
    -   `struct BrainRegion { std::string name; double intensity; std::vector<BrainRegion> subregions; };`
-   **Step 2: Enhance JSON Parser**:
    -   Update `json_logic.cpp` to parse a nested `subregions` array within each brain region object in the input JSON.
-   **Step 3: Refine Visualization Logic**:
    -   Modify `video_logic.cpp` to recursively process and render hierarchical regions.
    -   Subregions should be indented in the output to represent the hierarchy visually.
-   **Step 4: Update Sample Data**:
    -   Update `sample_input.json` and `expected_output.txt` to include and reflect hierarchical data for testing purposes.

### 1.2 Configuration File Support

**Objective**: Allow users to configure simulation parameters via an external file.

-   **Step 1: Design Configuration System**:
    -   Create `config.h` and `config.cpp` to manage loading and accessing configuration settings.
    -   The configuration will be a simple key-value format (e.g., `parameter=value`).
-   **Step 2: Implement Configuration Loading**:
    -   In `main.cpp`, add logic to load a `config.ini` file at startup.
    -   If the file doesn't exist, use default values.
-   **Step 3: Integrate Configuration Parameters**:
    -   Initial parameters will include `enable_color`, `smoothing_window_size`, and `layout_mode` (list/grid).

---

## Phase 2: Advanced Visualization

This phase aims to improve the visual output and user interaction.

### 2.1 ANSI Color-Coded Terminal Output

**Objective**: Add color to the terminal output for a richer visualization.

-   **Step 1: Define ANSI Color Codes**:
    -   Add a header file or constants within `video_logic.h` for ANSI color escape codes.
-   **Step 2: Implement Color Mapping**:
    -   In `video_logic.cpp`, modify the frame generation logic to map intensity values to specific colors.
-   **Step 3: Add Configuration Flag**:
    -   Use the `enable_color` parameter from the configuration file to toggle this feature on or off.

### 2.2 Grid/Matrix Brain Map Layouts

**Objective**: Implement a 2D grid layout for visualizing brain regions.

-   **Step 1: Design Grid Layout**:
    -   Conceptualize how brain regions will be mapped onto a 2D grid.
    -   The grid dimensions could be fixed or configurable.
-   **Step 2: Update Rendering Logic**:
    -   Create a new rendering function in `video_logic.cpp` for the grid layout.
    -   The function will take the list of brain regions and arrange them in the grid.
-   **Step 3: Add Layout Switching**:
    -   Use the `layout_mode` configuration parameter to switch between the default list view and the new grid view.

---

## Phase 3: Future Development (High-Level)

This section outlines future work that will be detailed in subsequent planning sessions.

-   **Temporal Smoothing and Decay Models**: Introduce more realistic temporal dynamics.
-   **Real-time Data Stream Processing**: Adapt the application for real-time data sources.
-   **Export Capabilities**: Allow exporting visualizations to common formats like PNG/JPEG without external libraries.
-   **AI/ML Integration**: Explore advanced AI-driven features like frame interpolation and generative visuals.
