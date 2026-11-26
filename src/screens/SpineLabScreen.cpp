#include "screens/SpineLabScreen.h"

#include "core/App.h"
#include "ui/CommonUI.h"
#include "ui/SpineParamsPanel.h"

#include <glad/gl.h>
#include <imgui.h>
#include <iostream>

#include "core/AssetPath.h"

void SpineLabScreen::initCubeMesh()
{
    const float vertices[] = {
        -0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,   0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,  -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,   0.5f, -0.5f,  0.5f,   0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,   0.5f, -0.5f, -0.5f,   0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,   0.5f,  0.5f,  0.5f,   0.5f, -0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,   0.5f, -0.5f, -0.5f,   0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,  -0.5f, -0.5f,  0.5f,  -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,   0.5f,  0.5f,  0.5f,   0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,  -0.5f,  0.5f, -0.5f,  -0.5f,  0.5f,  0.5f
    };

    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);

    glBindVertexArray(m_cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glDisableVertexAttribArray(1);

    glBindVertexArray(0);
}

void SpineLabScreen::onEnter()
{
    initCubeMesh();

    try {
        m_shader = std::make_unique<Shader>(resolveAssetPath("shaders/basic.vert"),
                                            resolveAssetPath("shaders/basic.frag"));
    }
    catch (const std::exception& e) {
        m_shader.reset();
        std::cerr << "Failed to load shaders: " << e.what() << std::endl;
    }

    m_camera.reset();
    m_camera.zoom(-15.0f);
    m_cameraInput.reset();
}

void SpineLabScreen::onExit()
{
    if (m_cubeVAO) glDeleteVertexArrays(1, &m_cubeVAO);
    if (m_cubeVBO) glDeleteBuffers(1, &m_cubeVBO);
    m_cubeVAO = 0;
    m_cubeVBO = 0;
}

void SpineLabScreen::update(float dt)
{
    m_camera.update(dt);

    const int width = m_app.framebufferWidth();
    const int height = m_app.framebufferHeight();
    if (height > 0) {
        m_camera.setAspectRatio(static_cast<float>(width) / static_cast<float>(height));
    }

    m_cameraInput.update(m_app.window(), m_camera);
}

void SpineLabScreen::render()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.10f, 0.10f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_shader) return;

    m_shader->use();

    m_shader->setMat4("uView", m_camera.viewMatrix());
    m_shader->setMat4("uProjection", m_camera.projectionMatrix());

    glBindVertexArray(m_cubeVAO);
    const auto& vertebrae = m_spineDeformer.getVertebrae();

    for (const auto& v : vertebrae) {
        m_shader->setMat4("uModel", v.getModelMatrix());
        if (v.id <= 4) {
            glVertexAttrib3f(1, 0.2f, 0.8f, 0.9f);
        }
        else if (v.id <= 16) {
            glVertexAttrib3f(1, 0.9f, 0.9f, 0.95f);
        }
        else {
            glVertexAttrib3f(1, 1.0f, 0.7f, 0.4f);
        }

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);
}

void SpineLabScreen::drawUI()
{
    ui::drawTopBar(m_app, name());

    if (ui::drawSpineParamsPanel(m_params)) {
        m_spineDeformer.applyCDeformation(m_params);
    }

	ImGui::Begin("Camera Controls");
	if (ImGui::Button("Front View (Coronal)")) {
		m_camera.requestPreset(CameraPreset::Front);
	}
	ImGui::SameLine();
	if (ImGui::Button("Side View (Sagital)")) {
		m_camera.requestPreset(CameraPreset::Side);
	}
	ImGui::SameLine();
	if (ImGui::Button("3D View / Angled View")) {
        m_camera.reset();
		m_camera.orbitBy(1.1f, 0.35f);
		m_camera.zoom(-15.0f);
	}
	ImGui::End();
}
