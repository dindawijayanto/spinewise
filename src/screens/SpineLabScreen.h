#pragma once

#include "core/Screen.h"
#include "spine/SpineDeformer.h"
#include "render/Shader.h"   
#include "camera/Camera.h"
#include "camera/CameraInput.h"
#include <memory>

// Owner: Diny (Spine) + Dealova (Camera/Render)
// Panel input SpineParams, spine prosedural, deformasi C/S, mode comparison.
class SpineLabScreen : public Screen
{
public:
    using Screen::Screen;
    const char* name() const override { return "Spine Lab"; }

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render() override;
    void drawUI() override;
private:
    void initCubeMesh(); // Helper untuk membuat mesh 1 kubus
    // Spine (Diny)
    Spinewise::SpineDeformer m_spineDeformer;
    // Render & Kamera (Dealova)
    std::unique_ptr<Shader> m_shader;
    OrbitCamera m_camera;
    OrbitCameraInput m_cameraInput;
    // Buffer OpenGL untuk kubus
    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;
};
