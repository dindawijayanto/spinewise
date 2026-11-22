#pragma once

struct GLFWwindow;
class OrbitCamera;

// Input mouse untuk OrbitCamera, dipakai ulang di semua screen 3D.
//   LMB drag : orbit
//   RMB drag : pan
//   Scroll   : zoom
// Input diabaikan kalau mouse sedang di atas panel ImGui.
class OrbitCameraInput
{
public:
    void update(GLFWwindow* window, OrbitCamera& camera);

    // Panggil saat screen dibuka supaya drag pertama tidak meloncat.
    void reset();

private:
    bool m_prevLeftDown = false;
    bool m_prevRightDown = false;
    double m_prevMouseX = 0.0;
    double m_prevMouseY = 0.0;
};
