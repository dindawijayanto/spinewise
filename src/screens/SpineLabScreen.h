#pragma once

#include "core/Screen.h"
#include "spine/SpineDeformer.h"
#include "render/Shader.h"
#include "camera/Camera.h"
#include "camera/CameraInput.h"
#include <memory>

enum class ComparisonMode
{
    None,      
    SideBySide, 
    Overlay,   
};

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
    void initCubeMesh();

    void renderReference(const glm::mat4& view, const glm::mat4& proj, float alpha = 1.0f);
    void renderDeformed(const glm::mat4& view, const glm::mat4& proj, float alpha = 1.0f);
    void renderVertebra(const Spinewise::Vertebra& v, bool useBase,
                        const glm::mat4& view, const glm::mat4& proj, float alpha);

    void renderSideBySide(int fbW, int fbH);
    void renderOverlay(int fbW, int fbH);
    void renderSingle(int fbW, int fbH);

    Spinewise::SpineDeformer m_spineDeformer;
    Spinewise::SpineParams m_params;

    std::unique_ptr<Shader> m_shader;
    OrbitCamera m_camera;
    OrbitCameraInput m_cameraInput;

    unsigned int m_cubeVAO = 0;
    unsigned int m_cubeVBO = 0;

    ComparisonMode m_comparisonMode = ComparisonMode::None;
    float m_overlayAlpha = 0.45f;
};
