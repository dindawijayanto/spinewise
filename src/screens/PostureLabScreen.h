#pragma once

#include "camera/Camera.h"
#include "camera/CameraInput.h"
#include "core/Screen.h"
#include "spine/SpineParams.h"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <memory>
#include <string>
#include <vector>

class Shader;

// Owner: Rafiq (Anatomy & Animation)
// Deformasi prosedural kerangka asli (bahu/thorax, pelvis, spine) + animasi keyframe + play/pause/replay.
class PostureLabScreen : public Screen
{
public:
    explicit PostureLabScreen(App& app);
    ~PostureLabScreen() override;

    const char* name() const override { return "Posture Lab"; }

    void onEnter() override;
    void onExit() override;
    void update(float dt) override;
    void render() override;
    void drawUI() override;

private:
    enum class AnatomyPart
    {
        Other = 0,
        Pelvis,
        Sacrum,
        Lumbar,
        Thoracic,
        Cervical,
        Skull,
        ThoraxShoulder,
        UpperLimb,
        LowerLimb
    };

    struct SkeletonVertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        float regionId;
    };

    struct BaseVertexData
    {
        glm::vec3 pos;
        glm::vec3 normal;
        float regionId;
        AnatomyPart part;
    };

    struct PostureKeyframe
    {
        float time = 0.0f; // 0.0 .. 1.0
        Spinewise::SpineParams params{};
        std::string label;
    };

    enum class AnimationPreset
    {
        CCurveProgression = 0,
        SCurveCompensation,
        AlignmentCycle
    };

    bool loadSkeletonBase(const std::string& objPath);
    void destroySkeletonBuffers();
    void updateDeformedMesh(const Spinewise::SpineParams& params);

    void loadPreset(AnimationPreset preset);
    Spinewise::SpineParams evaluateKeyframes(float normalizedTime) const;

    std::unique_ptr<Shader> m_shader;
    std::string m_loadError;

    OrbitCamera m_camera;
    OrbitCameraInput m_cameraInput;
    glm::mat4 m_modelMatrix{ 1.0f };

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    bool m_isLoaded = false;

    std::vector<BaseVertexData> m_baseVertices;
    std::vector<SkeletonVertex> m_deformedVertices;

    // Batas koordinat lokal model asli (dalam ruang OBJ sebelum rotasi -90 X)
    // Di OBJ Blender: X = kiri/kanan, Y = depan/belakang, Z = tinggi vertikal
    float m_pelvisPivotZ = 0.0f;
    float m_spineMinZ = 0.0f;
    float m_spineMaxZ = 1.0f;
    float m_shoulderPivotZ = 1.0f;
    float m_feetMinZ = -1.7f;

    // Keyframe & Playback State
    AnimationPreset m_currentPreset = AnimationPreset::CCurveProgression;
    std::vector<PostureKeyframe> m_keyframes;

    bool m_isPlaying = true;
    bool m_loop = true;
    float m_currentTime = 0.0f;
    float m_duration = 5.0f;
    float m_playbackSpeed = 1.0f;

    bool m_showLimbs = true;
    float m_lastEvaluatedPrimary = -1.0f;
    float m_lastEvaluatedSecondary = -1.0f;
    int m_lastEvaluatedPattern = -1;

    // Telemetri biomekanika frame aktif
    Spinewise::SpineParams m_activeParams{};
    float m_shoulderTiltDeg = 0.0f;
    float m_pelvisTiltDeg = 0.0f;
};