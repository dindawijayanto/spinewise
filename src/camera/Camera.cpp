#include "camera/Camera.h"

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <algorithm>
#include <cmath>

namespace
{
constexpr float kPi = 3.14159265358979323846f;

// Preset target orientations
constexpr float kYawFront = -kPi / 2.0f; 
constexpr float kYawSide  =  0.0f;

constexpr float kPitchFrontSide = 0.0f;
constexpr float kPitchTop = kPi / 2.0f - 0.0872664626f;

constexpr float kRotateSpeed = 0.005f;
constexpr float kPanSpeed    = 0.0015f;
constexpr float kZoomSpeed   = 0.5f;

constexpr float kTransitionDuration = 0.6f;

float smoothStep(float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float shortestAngleDelta(float from, float to)
{
    float delta = std::fmod(to - from, 2.0f * kPi);
    if (delta > kPi)
        delta -= 2.0f * kPi;
    else if (delta < -kPi)
        delta += 2.0f * kPi;
    return delta;
}
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

void OrbitCamera::update(float dt)
{
    if (!m_transitioning)
    {
        return;
    }

    m_transitionTime += dt;

    if (m_transitionTime >= m_transitionDuration)
    {
        m_yaw      = m_toYaw;
        m_pitch    = m_toPitch;
        m_distance = m_toDistance;
        m_transitioning = false;
        clampState();
        return;
    }

    const float rawT   = m_transitionTime / m_transitionDuration;
    const float easedT = smoothStep(rawT);

    m_yaw      = m_fromYaw + shortestAngleDelta(m_fromYaw, m_toYaw) * easedT;
    m_pitch    = m_fromPitch    + (m_toPitch    - m_fromPitch)    * easedT;
    m_distance = m_fromDistance + (m_toDistance - m_fromDistance) * easedT;

    clampState();
}

void OrbitCamera::rotate(float deltaX, float deltaY)
{
    cancelTransition();
    m_yaw   += deltaX * kRotateSpeed;
    m_pitch += -deltaY * kRotateSpeed;
    clampState();
}

void OrbitCamera::orbitBy(float yawRadians, float pitchRadians)
{
    cancelTransition();
    m_yaw   += yawRadians;
    m_pitch += pitchRadians;
    clampState();
}

void OrbitCamera::pan(float deltaX, float deltaY)
{
    cancelTransition();
    const float panScale = m_distance * kPanSpeed;
    m_target += (-right() * deltaX + up() * deltaY) * panScale;
}

void OrbitCamera::zoom(float delta)
{
    cancelTransition();
    m_distance -= delta * kZoomSpeed;
    clampState();
}

void OrbitCamera::reset()
{
    cancelTransition();
    m_target   = glm::vec3(0.0f, 0.0f, 0.0f);
    m_distance = 6.0f;
    m_yaw      = kYawFront;
    m_pitch    = 0.34906585f;
    clampState();
}

void OrbitCamera::requestPreset(CameraPreset preset)
{
    m_fromYaw      = m_yaw;
    m_fromPitch    = m_pitch;
    m_fromDistance = m_distance;

    switch (preset)
    {
    case CameraPreset::Front:
        m_toYaw      = kYawFront;
        m_toPitch    = kPitchFrontSide;
        m_toDistance = m_distance;
        break;

    case CameraPreset::Side:
        m_toYaw      = kYawSide;
        m_toPitch    = kPitchFrontSide;
        m_toDistance = m_distance;
        break;

    case CameraPreset::Top:
        m_toYaw      = m_yaw;
        m_toPitch    = kPitchTop;
        m_toDistance = m_distance;
        break;
    }

    m_transitionTime     = 0.0f;
    m_transitionDuration = kTransitionDuration;
    m_transitioning      = true;
}

void OrbitCamera::cancelTransition()
{
    m_transitioning = false;
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
    m_pitch    = std::clamp(m_pitch, m_minPitch, m_maxPitch);
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
