#include "camera/CameraInput.h"

#include "camera/Camera.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

void OrbitCameraInput::reset()
{
    m_prevLeftDown = false;
    m_prevRightDown = false;
}

void OrbitCameraInput::update(GLFWwindow* window, OrbitCamera& camera)
{
    if (!window)
    {
        return;
    }

    double mouseX = 0.0;
    double mouseY = 0.0;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    const float deltaX = static_cast<float>(mouseX - m_prevMouseX);
    const float deltaY = static_cast<float>(mouseY - m_prevMouseY);
    m_prevMouseX = mouseX;
    m_prevMouseY = mouseY;

    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
    {
        reset();
        return;
    }

    const bool leftDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool rightDown = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    // Delta baru dipakai mulai frame kedua drag, supaya tidak meloncat.
    if (leftDown && m_prevLeftDown)
    {
        camera.rotate(deltaX, deltaY);
    }
    if (rightDown && m_prevRightDown)
    {
        camera.pan(deltaX, deltaY);
    }
    m_prevLeftDown = leftDown;
    m_prevRightDown = rightDown;

    if (io.MouseWheel != 0.0f)
    {
        camera.zoom(io.MouseWheel);
    }
}
