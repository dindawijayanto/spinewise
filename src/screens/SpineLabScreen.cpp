#include "screens/SpineLabScreen.h"

#include "core/App.h"
#include "ui/CommonUI.h"
#include "ui/SpineParamsPanel.h"

#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    glDisableVertexAttribArray(1);

    glBindVertexArray(0);
}

void SpineLabScreen::onEnter(){
    initCubeMesh();
    try
    {
        m_shader = std::make_unique<Shader>(resolveAssetPath("shaders/basic.vert"),
                                            resolveAssetPath("shaders/basic.frag"));
    }
    catch (const std::exception& e)
    {
        m_shader.reset();
        std::cerr << "SpineLabScreen: failed to load shaders: " << e.what() << '\n';
    }

    m_camera.reset();
    m_camera.zoom(-15.0f);
    m_cameraInput.reset();
}

void SpineLabScreen::onExit()
{
    if (m_cubeVAO) { glDeleteVertexArrays(1, &m_cubeVAO); m_cubeVAO = 0; }
    if (m_cubeVBO) { glDeleteBuffers(1,     &m_cubeVBO);  m_cubeVBO = 0; }
}

void SpineLabScreen::update(float dt)
{
    m_camera.update(dt);
    m_spineDeformer.update(dt);

    m_highlightState = Spinewise::SpineHighlightEvaluator::evaluate(
        m_params, m_spineDeformer.getVertebrae(), m_highlightParams);

    const int w = m_app.framebufferWidth();
    const int h = m_app.framebufferHeight();

    const float viewW = (m_comparisonMode == ComparisonMode::SideBySide)
                        ? static_cast<float>(w) * 0.5f
                        : static_cast<float>(w);
    if (h > 0)
    {
        m_camera.setAspectRatio(viewW / static_cast<float>(h));
    }

    m_cameraInput.update(m_app.window(), m_camera);
}

void SpineLabScreen::renderVertebra(const Spinewise::Vertebra& v, bool useBase,
                                    const glm::mat4& view, const glm::mat4& proj,
                                    float alpha)
{
    glm::mat4 model(1.0f);
    const glm::vec3& pos = useBase ? v.basePosition : v.currentPosition;
    model = glm::translate(model, pos);
    model = glm::rotate(model, v.rotationEuler.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, v.rotationEuler.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, v.rotationEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, v.scale);

    m_shader->setMat4("uModel",      model);
    m_shader->setMat4("uView",       view);
    m_shader->setMat4("uProjection", proj);
    m_shader->setFloat("uAlpha",     alpha);

    // Apply Vertebra Highlight evaluation
    const auto& vInfo = (v.id >= 0 && v.id < static_cast<int>(m_highlightState.vertebraHighlights.size()))
                        ? m_highlightState.vertebraHighlights[v.id]
                        : Spinewise::VertebraHighlightInfo{};

    glVertexAttrib3f(1, vInfo.color.r, vInfo.color.g, vInfo.color.b);

    // Convex / Concave side tinting on vertebra
    const bool enableSide = m_highlightParams.enableSideVisualization && vInfo.isAffected;
    m_shader->setInt("uEnableSideViz", enableSide ? 1 : 0);

    const float convexDir = (m_highlightState.primaryConvexDirection == Spinewise::Direction::Right) ? -1.0f : 1.0f;
    m_shader->setFloat("uConvexDir", convexDir);
    m_shader->setVec3("uConvexColor", m_highlightParams.convexColor);
    m_shader->setVec3("uConcaveColor", m_highlightParams.concaveColor);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void SpineLabScreen::renderSideIndicators(const glm::mat4& view, const glm::mat4& proj)
{
    if (!m_highlightParams.enableSideVisualization) return;

    // Render 3D guide indicators on Left and Right of affected curve
    const float convexX = (m_highlightState.primaryConvexDirection == Spinewise::Direction::Right) ? -2.8f : 2.8f;
    const float concaveX = -convexX;

    m_shader->setInt("uEnableSideViz", 0);

    // Convex Side Guide Indicator
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(convexX, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.25f, 6.0f, 0.25f));
        m_shader->setMat4("uModel", model);
        m_shader->setMat4("uView", view);
        m_shader->setMat4("uProjection", proj);
        m_shader->setFloat("uAlpha", 0.75f);
        glVertexAttrib3f(1, m_highlightParams.convexColor.r, m_highlightParams.convexColor.g, m_highlightParams.convexColor.b);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // Concave Side Guide Indicator
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(concaveX, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.25f, 6.0f, 0.25f));
        m_shader->setMat4("uModel", model);
        m_shader->setMat4("uView", view);
        m_shader->setMat4("uProjection", proj);
        m_shader->setFloat("uAlpha", 0.75f);
        glVertexAttrib3f(1, m_highlightParams.concaveColor.r, m_highlightParams.concaveColor.g, m_highlightParams.concaveColor.b);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

void SpineLabScreen::renderReference(const glm::mat4& view, const glm::mat4& proj, float alpha)
{
    for (const auto& v : m_spineDeformer.getVertebrae())
        renderVertebra(v, /*useBase=*/true, view, proj, alpha);
}

void SpineLabScreen::renderDeformed(const glm::mat4& view, const glm::mat4& proj, float alpha)
{
    for (const auto& v : m_spineDeformer.getVertebrae())
        renderVertebra(v, /*useBase=*/false, view, proj, alpha);
}

void SpineLabScreen::renderSingle(int fbW, int fbH)
{
    glViewport(0, 0, fbW, fbH);
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    const glm::mat4 view = m_camera.viewMatrix();
    const glm::mat4 proj = m_camera.projectionMatrix();

    glBindVertexArray(m_cubeVAO);
    renderDeformed(view, proj, 1.0f);
    renderSideIndicators(view, proj);
    glBindVertexArray(0);
}

void SpineLabScreen::renderSideBySide(int fbW, int fbH)
{
    const int halfW = fbW / 2;

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    const glm::mat4 view = m_camera.viewMatrix();
    const glm::mat4 proj = m_camera.projectionMatrix();

    glBindVertexArray(m_cubeVAO);

    glViewport(0, 0, halfW, fbH);
    glScissor(0, 0, halfW, fbH);
    glEnable(GL_SCISSOR_TEST);
    glClear(GL_DEPTH_BUFFER_BIT); 
    renderReference(view, proj, 1.0f);

    glViewport(halfW, 0, fbW - halfW, fbH);
    glScissor(halfW, 0, fbW - halfW, fbH);
    glClear(GL_DEPTH_BUFFER_BIT);
    renderDeformed(view, proj, 1.0f);
    renderSideIndicators(view, proj);

    glDisable(GL_SCISSOR_TEST);
    glBindVertexArray(0);

    glViewport(0, 0, fbW, fbH);
}

void SpineLabScreen::renderOverlay(int fbW, int fbH)
{
    glViewport(0, 0, fbW, fbH);

    const glm::mat4 view = m_camera.viewMatrix();
    const glm::mat4 proj = m_camera.projectionMatrix();

    glBindVertexArray(m_cubeVAO);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    renderReference(view, proj, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE); 
    renderDeformed(view, proj, m_overlayAlpha);
    renderSideIndicators(view, proj);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindVertexArray(0);
}

void SpineLabScreen::render()
{
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.10f, 0.10f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_shader || m_cubeVAO == 0)
        return;

    m_shader->use();

    const int fbW = m_app.framebufferWidth();
    const int fbH = m_app.framebufferHeight();

    switch (m_comparisonMode)
    {
    case ComparisonMode::SideBySide:
        renderSideBySide(fbW, fbH);
        break;
    case ComparisonMode::Overlay:
        renderOverlay(fbW, fbH);
        break;
    case ComparisonMode::None:
    default:
        renderSingle(fbW, fbH);
        break;
    }
}

void SpineLabScreen::drawUI()
{
    ui::drawTopBar(m_app, name());

    if (ui::drawSpineParamsPanel(m_params, m_highlightParams, m_highlightState))
    {
        m_spineDeformer.applyDeformation(m_params, false);
    }

    ImGui::Begin("Camera Controls");

    if (ImGui::Button("Front View (Coronal)"))
        m_camera.requestPreset(CameraPreset::Front);
    ImGui::SameLine();
    if (ImGui::Button("Side View (Sagital)"))
        m_camera.requestPreset(CameraPreset::Side);
    ImGui::SameLine();
    if (ImGui::Button("3D View / Angled View"))
    {
        m_camera.reset();
        m_camera.orbitBy(1.1f, 0.35f);
        m_camera.zoom(-15.0f);
    }

    ImGui::Separator();
    if (ImGui::Button("Putar Animasi Transisi (1.5s)"))
        m_spineDeformer.applyDeformation(m_params, true);

    ImGui::Separator();
    ImGui::Text("Mode Perbandingan:");
    ImGui::Spacing();

    const bool isNone       = (m_comparisonMode == ComparisonMode::None);
    const bool isSideBySide = (m_comparisonMode == ComparisonMode::SideBySide);
    const bool isOverlay    = (m_comparisonMode == ComparisonMode::Overlay);

    if (isNone)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Normal"))
        m_comparisonMode = ComparisonMode::None;
    if (isNone)
        ImGui::PopStyleColor();

    ImGui::SameLine();

    if (isSideBySide)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Berdampingan"))
        m_comparisonMode = ComparisonMode::SideBySide;
    if (isSideBySide)
        ImGui::PopStyleColor();

    ImGui::SameLine();

    if (isOverlay)
        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
    if (ImGui::Button("Overlay"))
        m_comparisonMode = ComparisonMode::Overlay;
    if (isOverlay)
        ImGui::PopStyleColor();

    if (m_comparisonMode == ComparisonMode::Overlay)
    {
        ImGui::Spacing();
        ImGui::SliderFloat("Transparansi Overlay", &m_overlayAlpha, 0.1f, 0.9f, "%.2f");
    }

    if (m_comparisonMode == ComparisonMode::SideBySide)
    {
        ImGui::Spacing();
        ImGui::TextDisabled("Kiri: Normal   |   Kanan: Deformasi");
    }

    ImGui::End();
}
