#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

enum class CameraPreset
{
    Front,
    Side,
    Top,
};

class OrbitCamera
{
public:
    OrbitCamera();

    void setAspectRatio(float aspect);
    void setPerspective(float fovDegrees, float nearPlane, float farPlane);

    void update(float dt);

    void rotate(float deltaX, float deltaY);
    void orbitBy(float yawRadians, float pitchRadians);
    void pan(float deltaX, float deltaY);
    void zoom(float delta);

    void reset();

    void requestPreset(CameraPreset preset);

    void cancelTransition();

    bool isTransitioning() const { return m_transitioning; }

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;

    glm::vec3 position() const;
    glm::vec3 target() const { return m_target; }
    float distance() const { return m_distance; }

private:
    void clampState();
    glm::vec3 forward() const;
    glm::vec3 right() const;
    glm::vec3 up() const;

    glm::vec3 m_target{0.0f, 0.0f, 0.0f};
    float m_distance = 6.0f;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;

    float m_fovRadians = 0.785398163f; 
    float m_aspect = 16.0f / 9.0f;
    float m_near = 0.1f;
    float m_far = 100.0f;

    float m_minDistance = 1.5f;
    float m_maxDistance = 50.0f;
    float m_minPitch = -1.483529864f; 
    float m_maxPitch = 1.483529864f; 

    bool  m_transitioning = false;
    float m_transitionTime = 0.0f;
    float m_transitionDuration = 0.6f;

    float m_fromYaw = 0.0f;
    float m_fromPitch = 0.0f;
    float m_fromDistance = 6.0f;

    float m_toYaw = 0.0f;
    float m_toPitch = 0.0f;
    float m_toDistance = 6.0f;
};
