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

    void rotate(float deltaX, float deltaY);
    void pan(float deltaX, float deltaY);
    void zoom(float delta);

    void reset();
    void applyPreset(CameraPreset preset);

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

    float m_fovRadians = 45.0f;
    float m_aspect = 16.0f / 9.0f;
    float m_near = 0.1f;
    float m_far = 100.0f;

    float m_minDistance = 1.5f;
    float m_maxDistance = 50.0f;
    float m_minPitch = -1.483529864f; // -85 derajat
    float m_maxPitch = 1.483529864f;  // +85 derajat
};
