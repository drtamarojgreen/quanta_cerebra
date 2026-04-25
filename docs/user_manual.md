# QuantaCerebra User Manual

Welcome to **QuantaCerebra**, the highly specialized 3D simulation and visualization engine for the Greenhouse for Mental Health Development. This application translates abstract neuro-biological processes into immersive terminal-based environments.

---

## 1. Introduction: The Greenhouse Philosophy
The QuantaCerebra ecosystem treats the mind as an evolving greenhouse.
- **Growth (LTP):** Strengthening of neural connections.
- **Weather (Stochastic Modeling):** Neural noise and environmental flux.
- **Irrigation (Neurotransmitters):** The flow of chemical signals like Glutamate and GABA.

## 2. Core Features

### 2.1 Simulation Engine
- **Hierarchical Modeling:** Nested regions (e.g., Prefrontal Cortex within Frontal Lobe).
- **Temporal Physics:** Smoothing, Decay, Refractory Periods, and Synaptic Delays.
- **Analytical Intelligence:** Real-time FFT, PCA, and Granger Causality analysis.

### 2.2 Visualization
- **Layouts:** Standard List, 2D Grid, and 3D Pseudo-Projection.
- **Themes:** Dynamic, Ocean, Forest, and High-Contrast (Accessibility).
- **Exporters:** PNG, BMP, SVG, and animated GIF support.
- **Sonification:** Real-time MIDI generation and auditory feedback.

## 3. Configuration

### 3.1 `config.json`
```json
{
  "enable_color": true,
  "layout_mode": "3d",
  "theme": "ocean",
  "zoom": 1.0,
  "smoothing_window_size": 5,
  "activity_decay_rate": 0.1
}
```

### 3.2 CLI Commands
- `--3d`: Enable 3D Greenhouse projection.
- `--color`: Enable 256-color ANSI output.
- `--input=file.json`: Specify data source.
- `--api`: Start the background REST server on port 8080.
- `--s3`: Sync current state to simulated cloud storage.

## 4. Interactive Controls
While the simulation is running:
- `P`: Pause/Resume.
- `F`: Increase simulation speed.
- `U`: Hot-reload `config.json`.
- `Z/X`: Zoom in and out.

## 5. Technical Integration
QuantaCerebra provides a **C-API Wrapper** and **Library Mode** for integration into external research pipelines.
- `qc_init_simulation()`: Bootstrap the engine.
- `qc_process_frame()`: Feed JSON activity data.

---
*Constructed by Artsy Directora for the Greenhouse for Mental Health Development.*
