#include "screens/SkeletonExplorerScreen.h"

#include "core/App.h"
#include "core/AssetPath.h"
#include "render/Shader.h"
#include "ui/CommonUI.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <imgui.h>

#include <cstdio>
#include <exception>

SkeletonExplorerScreen::SkeletonExplorerScreen(App& app)
    : Screen(app)
{
}
SkeletonExplorerScreen::~SkeletonExplorerScreen() = default;
void SkeletonExplorerScreen::onEnter()
{
    createCubeBuffers();

    // Shader hilang/gagal compile jangan sampai membuat aplikasi crash:
    // tangkap error, tampilkan di UI, dan render() otomatis skip karena m_shader null.
    m_loadError.clear();
    try
    {
        m_shader = std::make_unique<Shader>(resolveAssetPath("shaders/basic.vert"),
                                            resolveAssetPath("shaders/basic.frag"));
    }
    catch (const std::exception& e)
    {
        m_shader.reset();
        m_loadError = e.what();
        std::fprintf(stderr, "[SkeletonExplorer] %s\n", e.what());
    }

    m_camera.reset();
    m_camera.setPerspective(45.0f, 0.1f, 100.0f);
    m_cameraInput.reset();
}

void SkeletonExplorerScreen::onExit()
{
    m_shader.reset();
    destroyCubeBuffers();
}

void SkeletonExplorerScreen::update(float /*dt*/)
{
    const int width = m_app.framebufferWidth();
    const int height = m_app.framebufferHeight();
    if (height > 0)
    {
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

    m_cameraInput.update(m_app.window(), m_camera);
}

void SkeletonExplorerScreen::render()
{
    // Warna latar navy sesuai PRD, lalu render cube pengujian.
    glClearColor(0.12f, 0.14f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_shader)
    {
        return;
    }

    m_shader->use();

    glm::mat4 model(1.0f);
    model = glm::rotate(model, static_cast<float>(glfwGetTime()) * 0.4f, glm::vec3(0.2f, 1.0f, 0.0f));

    m_shader->setMat4("uModel", model);
    m_shader->setMat4("uView", m_camera.viewMatrix());
    m_shader->setMat4("uProjection", m_camera.projectionMatrix());

    glBindVertexArray(m_cubeVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void SkeletonExplorerScreen::drawUI()
{
    ui::drawTopBar(m_app, name());

    ImGui::SetNextWindowPos(ImVec2(10.0f, 54.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.70f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("##CameraControls", nullptr, flags))
    {
        ImGui::TextUnformatted("Orbit Camera (Demo)");
        ImGui::Separator();
        ImGui::TextUnformatted("LMB drag: Orbit");
        ImGui::TextUnformatted("RMB drag: Pan");
        ImGui::TextUnformatted("Scroll: Zoom");
        ImGui::Spacing();

        if (ImGui::Button("Front"))
            m_camera.applyPreset(CameraPreset::Front);
        ImGui::SameLine();
        if (ImGui::Button("Side"))
            m_camera.applyPreset(CameraPreset::Side);
        ImGui::SameLine();
        if (ImGui::Button("Top"))
            m_camera.applyPreset(CameraPreset::Top);

        if (ImGui::Button("Reset Camera", ImVec2(-1.0f, 0.0f)))
            m_camera.reset();

        ImGui::TextDisabled("Distance: %.2f", m_camera.distance());

        if (!m_loadError.empty())
        {
            ImGui::Spacing();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Gagal memuat shader:");
            ImGui::TextWrapped("%s", m_loadError.c_str());
        }
    }
    ImGui::End();
}

void SkeletonExplorerScreen::createCubeBuffers()
{
    if (m_cubeVao != 0 || m_cubeVbo != 0)
    {
        return;
    }

    // Posisi + warna vertex untuk cube 1x1x1 (36 vertex, tanpa EBO).
    static const float vertices[] = {
        // back face (z-)
        -0.5f, -0.5f, -0.5f, 0.9f, 0.2f, 0.2f,  0.5f, -0.5f, -0.5f, 0.9f, 0.2f, 0.2f,
         0.5f,  0.5f, -0.5f, 0.9f, 0.2f, 0.2f,  0.5f,  0.5f, -0.5f, 0.9f, 0.2f, 0.2f,
        -0.5f,  0.5f, -0.5f, 0.9f, 0.2f, 0.2f, -0.5f, -0.5f, -0.5f, 0.9f, 0.2f, 0.2f,

        // front face (z+)
        -0.5f, -0.5f,  0.5f, 0.2f, 0.8f, 0.9f,  0.5f, -0.5f,  0.5f, 0.2f, 0.8f, 0.9f,
         0.5f,  0.5f,  0.5f, 0.2f, 0.8f, 0.9f,  0.5f,  0.5f,  0.5f, 0.2f, 0.8f, 0.9f,
        -0.5f,  0.5f,  0.5f, 0.2f, 0.8f, 0.9f, -0.5f, -0.5f,  0.5f, 0.2f, 0.8f, 0.9f,

        // left face (x-)
        -0.5f,  0.5f,  0.5f, 0.3f, 0.9f, 0.4f, -0.5f,  0.5f, -0.5f, 0.3f, 0.9f, 0.4f,
        -0.5f, -0.5f, -0.5f, 0.3f, 0.9f, 0.4f, -0.5f, -0.5f, -0.5f, 0.3f, 0.9f, 0.4f,
        -0.5f, -0.5f,  0.5f, 0.3f, 0.9f, 0.4f, -0.5f,  0.5f,  0.5f, 0.3f, 0.9f, 0.4f,

        // right face (x+)
         0.5f,  0.5f,  0.5f, 0.9f, 0.7f, 0.2f,  0.5f,  0.5f, -0.5f, 0.9f, 0.7f, 0.2f,
         0.5f, -0.5f, -0.5f, 0.9f, 0.7f, 0.2f,  0.5f, -0.5f, -0.5f, 0.9f, 0.7f, 0.2f,
         0.5f, -0.5f,  0.5f, 0.9f, 0.7f, 0.2f,  0.5f,  0.5f,  0.5f, 0.9f, 0.7f, 0.2f,

        // bottom face (y-)
        -0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.9f,  0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.9f,
         0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.9f,  0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.9f,
        -0.5f, -0.5f,  0.5f, 0.7f, 0.3f, 0.9f, -0.5f, -0.5f, -0.5f, 0.7f, 0.3f, 0.9f,

        // top face (y+)
        -0.5f,  0.5f, -0.5f, 0.2f, 0.7f, 1.0f,  0.5f,  0.5f, -0.5f, 0.2f, 0.7f, 1.0f,
         0.5f,  0.5f,  0.5f, 0.2f, 0.7f, 1.0f,  0.5f,  0.5f,  0.5f, 0.2f, 0.7f, 1.0f,
        -0.5f,  0.5f,  0.5f, 0.2f, 0.7f, 1.0f, -0.5f,  0.5f, -0.5f, 0.2f, 0.7f, 1.0f,
    };

    glGenVertexArrays(1, &m_cubeVao);
    glGenBuffers(1, &m_cubeVbo);

    glBindVertexArray(m_cubeVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    constexpr GLsizei stride = static_cast<GLsizei>(6 * sizeof(float));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SkeletonExplorerScreen::destroyCubeBuffers()
{
    if (m_cubeVbo != 0)
    {
        glDeleteBuffers(1, &m_cubeVbo);
        m_cubeVbo = 0;
    }
    if (m_cubeVao != 0)
    {
        glDeleteVertexArrays(1, &m_cubeVao);
        m_cubeVao = 0;
    }
}
