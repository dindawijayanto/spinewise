#pragma once

#include "core/Screen.h"

#include "camera/Camera.h"
#include "camera/CameraInput.h"

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <memory>
#include <string>

class Shader;

enum class SpineRegionId : int
{
    None = 0,
    Cervical = 1,
    Thoracic = 2,
    Lumbar = 3
};

struct SpineRegionInfo
{
    SpineRegionId id = SpineRegionId::None;
    const char* name = "";
    const char* vertebraeRange = "";
    const char* description = "";
    glm::vec3 minBounds{ 0.0f };
    glm::vec3 maxBounds{ 0.0f };
};

// Owner: Rafiq (Anatomy & Animation)
// Render skeleton OBJ, orbit camera, highlight region spine, ray picking.
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
    bool loadSkeletonOBJ(const std::string& objPath);
    void destroySkeletonBuffers();
    void setupRegionBounds();

    SpineRegionId pickRegion(double mouseX, double mouseY, int viewportW, int viewportH) const;
    static glm::vec3 screenToWorldRay(double mouseX, double mouseY, int screenW, int screenH,
        const glm::mat4& view, const glm::mat4& proj);
    static bool intersectRayAABB(const glm::vec3& rayOrig, const glm::vec3& rayDir,
        const glm::vec3& boxMin, const glm::vec3& boxMax,
        float& tHit);

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    int m_vertexCount = 0;
    bool m_isLoaded = false;

    std::unique_ptr<Shader> m_shader;
    std::string m_loadError; // ditampilkan di UI kalau shader / OBJ gagal dimuat
    OrbitCamera m_camera;
    OrbitCameraInput m_cameraInput;

    glm::mat4 m_modelMatrix{ 1.0f };

    // Rentang Y terkalibrasi untuk Skeletons.obj (skala [-1.8, +1.8])
    float m_cervicalMinY = 1.05f;
    float m_cervicalMaxY = 1.35f;

    float m_thoracicMinY = 0.42f;
    float m_thoracicMaxY = 1.05f;

    float m_lumbarMinY = 0.10f;
    float m_lumbarMaxY = 0.42f;

    glm::vec2 m_spineCenterXZ = glm::vec2(0.0f, 0.0f);
    float m_spineRadiusXZ = 0.22f;

    std::array<SpineRegionInfo, 3> m_regions{};
    SpineRegionId m_hoveredRegion = SpineRegionId::None;
    SpineRegionId m_selectedRegion = SpineRegionId::None;

    bool m_wasMouseDown = false;
    double m_pressX = 0.0;
    double m_pressY = 0.0;
};