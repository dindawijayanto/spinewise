#pragma once

#include "core/Screen.h"

#include "camera/Camera.h"
#include "camera/CameraInput.h"

#include <memory>
#include <string>

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

    unsigned int m_cubeVao = 0;
    unsigned int m_cubeVbo = 0;

    std::unique_ptr<Shader> m_shader;
    std::string m_loadError; // ditampilkan di UI kalau shader gagal dimuat
    OrbitCamera m_camera;
    OrbitCameraInput m_cameraInput;
};
