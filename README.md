# SPINEWISE

**An Interactive 3D Scoliosis Awareness & Learning Application**

SPINEWISE is an educational, real-time 3D graphics application designed to help users understand the structure of the human spine and how scoliosis affects it. Built from scratch using **C++17 and OpenGL 3.3**, this application generates a procedural 3D spine and deforms it in real-time based on user-entered scoliosis parameters (such as curve region, direction, pattern, and Cobb angle).

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

1. **Clone the repository:**
   Make sure Git LFS is installed before cloning to ensure 3D models are downloaded correctly.
   ```bash
   git lfs install
   git clone <repository-url>
   cd spinewise
   ```

2. **Generate the build files using CMake:**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

3. **Compile the project:**
   ```bash
   cmake --build .
   ```

4. **Run the application:**
   The executable will be generated inside the `build/` directory (or `build/Debug`/`build/Release` on Windows). Ensure you run the application from the root directory so the executable can correctly locate the `assets/` and `shaders/` folders.

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