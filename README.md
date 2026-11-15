# SPINEWISE 🦴

**An Interactive 3D Scoliosis Awareness & Learning Application**

SPINEWISE is an interactive 3D educational application designed to help users understand the structure of the human spine and how scoliosis changes it in three dimensions. By entering parameters from a standard examination report (curve region, direction, pattern, and Cobb angle), users can visualize a procedurally generated 3D spine bending to represent that curve, side-by-side with a normal reference spine.


> **Disclaimer:** SPINEWISE is an educational visualization tool. It does not diagnose scoliosis, interpret X-ray images, or prescribe exercises.

## ✨ Features

*   **Spine Lab:** Procedurally generate and deform a 3D spine based on clinical parameters. Compare the simulated scoliotic spine side-by-side with a healthy reference spine.
*   **Skeleton Explorer:** Explore a full 3D human skeleton to understand where the spine sits within the body, featuring interactive highlighted regions.
*   **Posture Lab:** Watch a procedural 3D animation demonstrating how a lateral spinal curve relates to visible posture changes.
*   **Fully Offline:** Runs entirely as a standalone desktop application with no internet connectivity or accounts required.

## 🛠️ Tech Stack

This project is built without a commercial game engine, focusing on custom graphics programming and rendering pipelines.

*   **Language:** C++17
*   **Graphics API:** OpenGL 3.3 Core Profile
*   **Windowing & Input:** GLFW
*   **OpenGL Loader:** GLAD
*   **Mathematics:** GLM
*   **User Interface:** Dear ImGui
*   **Asset Loading:** tinyobjloader (OBJ files), nlohmann/json (educational data)
*   **Build System:** CMake

## 🚀 Getting Started

### Prerequisites

To build and run this project locally, you will need:
*   A C++17 compatible compiler (e.g., MSVC, MinGW-w64, or GCC).
*   [CMake](https://cmake.org/download/) (version 3.15 or higher).
*   [Git LFS](https://git-lfs.com/) (Required to download the 3D `.obj` assets).

### Build Instructions

#### Windows

1. Clone the repository:

   ```bash
   git clone https://github.com/dindawijayanto/spinewise.git
   cd SPINEWISE
   ```

2. Generate the build files using CMake:

   ```bash
   cmake -B build
   ```

3. Compile the project:

   ```bash
   cmake --build build --config Release
   ```

4. Run the executable located in the `build/Release` (or `build`) directory.

#### macOS

1. Install Xcode Command Line Tools if you haven't already:

   ```bash
   xcode-select --install
   ```

2. Clone and navigate to the repository:

   ```bash
   git clone https://github.com/dindawijayanto/spinewise.git
   cd SPINEWISE
   ```

3. Generate build files and compile:

   ```bash
   cmake -B build
   cmake --build build
   ```

4. Run the executable:

   ```bash
   ./build/SPINEWISE
   ```

#### Linux

1. Install essential build tools and OpenGL dependencies based on your distribution:

   **Ubuntu/Debian (`apt`):**
   ```bash
   sudo apt update
   sudo apt install build-essential cmake libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev
   ```

   **Fedora (`dnf`):**
   ```bash
   sudo dnf groupinstall "Development Tools"
   sudo dnf install cmake libX11-devel libXrandr-devel libXi-devel mesa-libGL-devel mesa-libGLU-devel libXcursor-devel libXinerama-devel wayland-devel libxkbcommon-devel
   ```

   **Arch Linux (`pacman`):**
   ```bash
   sudo pacman -S base-devel cmake libx11 libxrandr libxi libglvnd glu libxcursor libxinerama wayland libxkbcommon
   ```

2. Clone and navigate to the repository:

   ```bash
   git clone https://github.com/dindawijayanto/spinewise.git
   cd SPINEWISE
   ```

3. Generate build files and compile:

   ```bash
   cmake -B build
   cmake --build build
   ```

4. Run the executable:

   ```bash
   ./build/SPINEWISE
   ```

## 📂 Repository Structure

```
spinewise/
├── assets/          # 3D models (.obj) and static educational text (.json)
├── external/        # Vendored third-party libraries (GLFW, ImGui, GLM, etc.)
├── shaders/         # Custom GLSL vertex and fragment shaders
├── src/             # Core C++ source code
│   ├── MainMenu/    # State machine and UI navigation
│   ├── SkeletonExplorer/
│   ├── SpineLab/    # Procedural generation and deformation logic
│   └── PostureLab/
├── CMakeLists.txt   # CMake configuration
└── README.md
```

## 📜 Credits & Acknowledgments

*   **Developed by:** Team "Ayo Bikin Roblok" as the Final Project for Computer Graphics.
*   **3D Assets:** Skeleton and vertebra models adapted from [Z-Anatomy / BodyParts3D] (licensed under CC BY-SA 4.0 or relevant license).