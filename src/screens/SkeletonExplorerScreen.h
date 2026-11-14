#pragma once

#include "core/Screen.h"

#include "camera/Camera.h"

#include <GLFW/glfw3.h>

#include <memory>

class Shader;

// Owner: Rafiq (Anatomy & Animation)
// Render skeleton OBJ, orbit camera, highlight region spine.
class SkeletonExplorerScreen : public Screen
{
public:
    explicit SkeletonExplorerScreen(App& app);
    ~SkeletonExplorerScreen() override;

    const char* name() const override { return "Skeleton Explorer"; }

    SkeletonExplorerScreen(const SkeletonExplorerScreen&) = delete;
    SkeletonExplorerScreen& operator=(const SkeletonExplorerScreen&) = delete;
    SkeletonExplorerScreen(SkeletonExplorerScreen&&) = delete;
    SkeletonExplorerScreen& operator=(SkeletonExplorerScreen&&) = delete;

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render() override;
    void drawUI() override;

private:
    void createCubeBuffers();
    void destroyCubeBuffers();
    void handleCameraInput();

    unsigned int m_cubeVao = 0;
    unsigned int m_cubeVbo = 0;

    std::unique_ptr<Shader> m_shader;
    OrbitCamera m_camera;

    bool m_prevLeftMouseDown = false;
    bool m_prevRightMouseDown = false;
    double m_prevMouseX = 0.0;
    double m_prevMouseY = 0.0;

    GLFWwindow* m_window = nullptr;
};
