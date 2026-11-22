#include "camera/Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>

namespace
{
constexpr float kPi = 3.14159265358979323846f;
constexpr float kYawFront = -kPi / 2.0f;
constexpr float kYawSide = 0.0f;
constexpr float kPitchFrontSide = 0.0f;
constexpr float kPitchTop = kPi / 2.0f - 0.0872664626f; // 85 derajat

constexpr float kRotateSpeed = 0.005f;
constexpr float kPanSpeed = 0.0015f;
constexpr float kZoomSpeed = 0.5f;
} 

OrbitCamera::OrbitCamera()
{
    m_fovRadians = glm::radians(45.0f);
    reset();
}

void OrbitCamera::setAspectRatio(float aspect)
{
    if (aspect > 0.0f)
    {
        m_aspect = aspect;
    }
}

void OrbitCamera::setPerspective(float fovDegrees, float nearPlane, float farPlane)
{
    m_fovRadians = glm::radians(fovDegrees);
    m_near = nearPlane;
    m_far = farPlane;
}

void OrbitCamera::rotate(float deltaX, float deltaY)
{
    m_yaw += deltaX * kRotateSpeed;
    m_pitch += -deltaY * kRotateSpeed;
    clampState();
}

void OrbitCamera::orbitBy(float yawRadians, float pitchRadians)
{
    m_yaw += yawRadians;
    m_pitch += pitchRadians;
    clampState();
}

void OrbitCamera::pan(float deltaX, float deltaY)
{
    const float panScale = m_distance * kPanSpeed;
    m_target += (-right() * deltaX + up() * deltaY) * panScale;
}

void OrbitCamera::zoom(float delta)
{
    m_distance -= delta * kZoomSpeed;
    clampState();
}

void OrbitCamera::reset()
{
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_distance = 6.0f;
    m_yaw = kYawFront;
    m_pitch = 0.34906585f; // 20 derajat agar kedalaman objek langsung terasa
    clampState();
}

void OrbitCamera::applyPreset(CameraPreset preset)
{
    switch (preset)
    {
    case CameraPreset::Front:
        m_yaw = kYawFront;
        m_pitch = kPitchFrontSide;
        break;
    case CameraPreset::Side:
        m_yaw = kYawSide;
        m_pitch = kPitchFrontSide;
        break;
    case CameraPreset::Top:
        m_pitch = kPitchTop;
        break;
    }
    clampState();
}

glm::mat4 OrbitCamera::viewMatrix() const
{
    return glm::lookAt(position(), m_target, up());
}

glm::mat4 OrbitCamera::projectionMatrix() const
{
    return glm::perspective(m_fovRadians, m_aspect, m_near, m_far);
}

glm::vec3 OrbitCamera::position() const
{
    const float cosPitch = glm::cos(m_pitch);
    const glm::vec3 offset(
        m_distance * cosPitch * glm::cos(m_yaw),
        m_distance * glm::sin(m_pitch),
        m_distance * cosPitch * glm::sin(m_yaw));
    return m_target + offset;
}

void OrbitCamera::clampState()
{
    m_distance = std::clamp(m_distance, m_minDistance, m_maxDistance);
    m_pitch = std::clamp(m_pitch, m_minPitch, m_maxPitch);
}

glm::vec3 OrbitCamera::forward() const
{
    return glm::normalize(m_target - position());
}

glm::vec3 OrbitCamera::right() const
{
    static const glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    return glm::normalize(glm::cross(forward(), worldUp));
}

glm::vec3 OrbitCamera::up() const
{
    return glm::normalize(glm::cross(right(), forward()));
}
