#include "screens/SkeletonExplorerScreen.h"

#include "core/App.h"
#include "render/Shader.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <tiny_obj_loader.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <vector>

namespace
{
    constexpr float kSidePanelRatio = 0.34f; // 34% kiri untuk panel UI, 66% kanan untuk viewport 3D

    struct SkeletonVertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        float regionId;
    };

    SpineRegionId detectRegionFromName(const std::string& rawName)
    {
        std::string lower = rawName;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        if (lower.find("cervical") != std::string::npos)
            return SpineRegionId::Cervical;
        if (lower.find("thoracic") != std::string::npos)
            return SpineRegionId::Thoracic;
        if (lower.find("lumbar") != std::string::npos)
            return SpineRegionId::Lumbar;

        return SpineRegionId::None;
    }

    void resetExplorerCamera(OrbitCamera& camera, OrbitCameraInput& input, glm::mat4& modelMatrix)
    {
        camera.reset();
        input.reset();

        // Hadapkan belakang (tampak punggung/spine) dan perbesar sedikit agar lebih jelas
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(1.15f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = model;
    }
} // namespace

SkeletonExplorerScreen::SkeletonExplorerScreen(App& app)
    : Screen(app)
{
    setupRegionBounds();
}

SkeletonExplorerScreen::~SkeletonExplorerScreen()
{
    destroySkeletonBuffers();
}

void SkeletonExplorerScreen::onEnter()
{
    m_loadError.clear();
    m_selectedRegion = SpineRegionId::None;
    m_hoveredRegion = SpineRegionId::None;
    m_wasMouseDown = false;

    try
    {
        m_shader = std::make_unique<Shader>("shaders/skeleton.vert", "shaders/skeleton.frag");
    }
    catch (const std::exception& e)
    {
        m_loadError = std::string("Gagal memuat shader skeleton: ") + e.what();
        return;
    }

    if (!m_isLoaded)
    {
        if (!loadSkeletonOBJ("assets/models/Skeletons.obj"))
        {
            if (m_loadError.empty())
            {
                m_loadError = "Gagal memuat assets/models/Skeletons.obj (pastikan Git LFS sudah di-pull).";
            }
            return;
        }
    }

    resetExplorerCamera(m_camera, m_cameraInput, m_modelMatrix);
}

void SkeletonExplorerScreen::onExit()
{
    GLFWwindow* win = m_app.window();
    if (win)
    {
        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(win, &fbW, &fbH);
        if (fbW > 0 && fbH > 0)
        {
            glViewport(0, 0, fbW, fbH);
        }
    }

    destroySkeletonBuffers();
    m_shader.reset();
}

void SkeletonExplorerScreen::update(float dt)
{
    (void)dt;

    GLFWwindow* win = m_app.window();
    if (!win)
    {
        return;
    }

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(win, &fbW, &fbH);
    if (fbW <= 0 || fbH <= 0)
    {
        return;
    }

    const int panelFbW = static_cast<int>(static_cast<float>(fbW) * kSidePanelRatio);
    const int viewFbW = std::max(1, fbW - panelFbW);

    m_camera.setAspectRatio(static_cast<float>(viewFbW) / static_cast<float>(fbH));
    m_cameraInput.update(win, m_camera);

    if (ImGui::GetIO().WantCaptureMouse)
    {
        m_hoveredRegion = SpineRegionId::None;
        return;
    }

    int winW = 0, winH = 0;
    glfwGetWindowSize(win, &winW, &winH);
    double mouseX = 0.0, mouseY = 0.0;
    glfwGetCursorPos(win, &mouseX, &mouseY);

    const double panelWinW = static_cast<double>(winW) * static_cast<double>(kSidePanelRatio);
    const double viewWinW = std::max(1.0, static_cast<double>(winW) - panelWinW);
    const double localMouseX = mouseX - panelWinW;

    if (localMouseX < 0.0 || localMouseX > viewWinW || mouseY < 0.0 || mouseY > static_cast<double>(winH))
    {
        m_hoveredRegion = SpineRegionId::None;
    }
    else
    {
        m_hoveredRegion = pickRegion(localMouseX, mouseY, static_cast<int>(viewWinW), winH);
    }

    const bool mouseDown = (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    if (mouseDown && !m_wasMouseDown)
    {
        m_pressX = mouseX;
        m_pressY = mouseY;
    }
    else if (!mouseDown && m_wasMouseDown)
    {
        const double moveDist = std::hypot(mouseX - m_pressX, mouseY - m_pressY);
        if (moveDist < 5.0 && m_hoveredRegion != SpineRegionId::None)
        {
            m_selectedRegion = m_hoveredRegion;
        }
    }
    m_wasMouseDown = mouseDown;
}

void SkeletonExplorerScreen::render()
{
    GLFWwindow* win = m_app.window();
    if (!win || !m_isLoaded || !m_shader)
    {
        return;
    }

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(win, &fbW, &fbH);
    if (fbW <= 0 || fbH <= 0)
    {
        return;
    }

    const int panelFbW = static_cast<int>(static_cast<float>(fbW) * kSidePanelRatio);
    const int viewFbW = std::max(1, fbW - panelFbW);
    glViewport(panelFbW, 0, viewFbW, fbH);

    glEnable(GL_DEPTH_TEST);

    m_shader->use();
    m_shader->setMat4("uModel", m_modelMatrix);
    m_shader->setMat4("uView", m_camera.viewMatrix());
    m_shader->setMat4("uProjection", m_camera.projectionMatrix());

    const glm::vec3 camPos = m_camera.position();
    m_shader->setVec3("uViewPos", camPos);
    m_shader->setVec3("uLightPos", camPos + glm::vec3(2.0f, 3.0f, 2.5f));

    m_shader->setInt("uHoveredRegion", static_cast<int>(m_hoveredRegion));
    m_shader->setInt("uSelectedRegion", static_cast<int>(m_selectedRegion));

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    glBindVertexArray(0);

    glViewport(0, 0, fbW, fbH);
}

void SkeletonExplorerScreen::drawUI()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float panelW = vp->Size.x * kSidePanelRatio;

    ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelW, vp->Size.y), ImGuiCond_Always);

    const ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Skeleton Explorer", nullptr, panelFlags);
    if (!m_loadError.empty())
    {
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "%s", m_loadError.c_str());
        ImGui::Spacing();
        if (ImGui::Button("Kembali ke Menu", ImVec2(-1.0f, 32.0f)))
        {
            m_app.switchTo(ScreenId::Menu);
        }
        ImGui::End();
        return;
    }

    ImGui::TextWrapped("Eksplorasi anatomi tulang belakang (spine) pada kerangka 3D.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Regio Tulang Belakang:");
    ImGui::Spacing();

    ImGui::ColorButton("##cerv", ImVec4(0.18f, 0.80f, 0.92f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine();
    if (ImGui::Selectable("Cervical (C1 - C7)", m_selectedRegion == SpineRegionId::Cervical))
        m_selectedRegion = SpineRegionId::Cervical;

    ImGui::ColorButton("##thor", ImVec4(0.22f, 0.84f, 0.58f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine();
    if (ImGui::Selectable("Thoracic (T1 - T12)", m_selectedRegion == SpineRegionId::Thoracic))
        m_selectedRegion = SpineRegionId::Thoracic;

    ImGui::ColorButton("##lumb", ImVec4(0.95f, 0.74f, 0.22f, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(14, 14));
    ImGui::SameLine();
    if (ImGui::Selectable("Lumbar (L1 - L5)", m_selectedRegion == SpineRegionId::Lumbar))
        m_selectedRegion = SpineRegionId::Lumbar;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (m_selectedRegion != SpineRegionId::None)
    {
        const size_t idx = static_cast<size_t>(static_cast<int>(m_selectedRegion) - 1);
        const auto& info = m_regions[idx];

        ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.15f, 1.0f), "%s", info.name);
        ImGui::Text("Ruas: %s", info.vertebraeRange);
        ImGui::Spacing();
        ImGui::TextWrapped("%s", info.description);
    }
    else
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("Klik salah satu regio tulang belakang pada model 3D atau daftar di atas untuk melihat deskripsi.");
        ImGui::PopStyleColor();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextDisabled("Kontrol Kamera:");
    ImGui::BulletText("Drag Kiri: Putar model");
    ImGui::BulletText("Drag Kanan: Geser (Pan)");
    ImGui::BulletText("Scroll: Zoom in / out");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Reset View", ImVec2(-1.0f, 32.0f)))
    {
        resetExplorerCamera(m_camera, m_cameraInput, m_modelMatrix);
        m_selectedRegion = SpineRegionId::None;
    }

    ImGui::Spacing();
    if (ImGui::Button("Kembali ke Menu", ImVec2(-1.0f, 32.0f)))
    {
        m_app.switchTo(ScreenId::Menu);
    }
    ImGui::End();
}

bool SkeletonExplorerScreen::loadSkeletonOBJ(const std::string& objPath)
{
    tinyobj::ObjReaderConfig readerConfig;
    readerConfig.triangulate = true;
    readerConfig.mtl_search_path = "assets/models/";

    tinyobj::ObjReader reader;
    if (!reader.ParseFromFile(objPath, readerConfig))
    {
        m_loadError = reader.Error();
        return false;
    }

    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();
    if (attrib.vertices.empty() || shapes.empty())
    {
        return false;
    }

    glm::vec3 rawMin(std::numeric_limits<float>::max());
    glm::vec3 rawMax(std::numeric_limits<float>::lowest());

    for (size_t i = 0; i + 2 < attrib.vertices.size(); i += 3)
    {
        glm::vec3 v(attrib.vertices[i], attrib.vertices[i + 1], attrib.vertices[i + 2]);
        rawMin = glm::min(rawMin, v);
        rawMax = glm::max(rawMax, v);
    }

    const glm::vec3 rawCenter = 0.5f * (rawMin + rawMax);
    const glm::vec3 rawExtent = rawMax - rawMin;
    const float maxAxis = std::max({ rawExtent.x, rawExtent.y, rawExtent.z, 0.0001f });
    const float normalizeScale = 3.4f / maxAxis;

    glm::vec3 regMin[3] = {
        glm::vec3(std::numeric_limits<float>::max()),
        glm::vec3(std::numeric_limits<float>::max()),
        glm::vec3(std::numeric_limits<float>::max())
    };
    glm::vec3 regMax[3] = {
        glm::vec3(std::numeric_limits<float>::lowest()),
        glm::vec3(std::numeric_limits<float>::lowest()),
        glm::vec3(std::numeric_limits<float>::lowest())
    };
    bool hasRegBounds[3] = { false, false, false };

    std::vector<SkeletonVertex> vertices;
    for (const auto& shape : shapes)
    {
        const SpineRegionId shapeRegion = detectRegionFromName(shape.name);
        const float regionFloat = static_cast<float>(static_cast<int>(shapeRegion));

        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
        {
            const size_t fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);
            if (fv != 3)
            {
                indexOffset += fv;
                continue;
            }

            SkeletonVertex tri[3];
            bool hasNormals = true;

            for (size_t v = 0; v < 3; ++v)
            {
                const tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];
                const size_t vIdx = static_cast<size_t>(3 * idx.vertex_index);

                glm::vec3 pos(
                    attrib.vertices[vIdx + 0],
                    attrib.vertices[vIdx + 1],
                    attrib.vertices[vIdx + 2]);

                tri[v].pos = (pos - rawCenter) * normalizeScale;
                tri[v].regionId = regionFloat;

                if (shapeRegion != SpineRegionId::None)
                {
                    const size_t rIdx = static_cast<size_t>(static_cast<int>(shapeRegion) - 1);
                    regMin[rIdx] = glm::min(regMin[rIdx], tri[v].pos);
                    regMax[rIdx] = glm::max(regMax[rIdx], tri[v].pos);
                    hasRegBounds[rIdx] = true;
                }

                if (idx.normal_index >= 0 && static_cast<size_t>(3 * idx.normal_index + 2) < attrib.normals.size())
                {
                    const size_t nIdx = static_cast<size_t>(3 * idx.normal_index);
                    tri[v].normal = glm::normalize(glm::vec3(
                        attrib.normals[nIdx + 0],
                        attrib.normals[nIdx + 1],
                        attrib.normals[nIdx + 2]));
                }
                else
                {
                    hasNormals = false;
                }
            }

            if (!hasNormals)
            {
                const glm::vec3 e1 = tri[1].pos - tri[0].pos;
                const glm::vec3 e2 = tri[2].pos - tri[0].pos;
                const glm::vec3 crossN = glm::cross(e1, e2);
                const glm::vec3 faceNormal = (glm::length(crossN) > 1e-6f)
                    ? glm::normalize(crossN)
                    : glm::vec3(0.0f, 1.0f, 0.0f);
                tri[0].normal = faceNormal;
                tri[1].normal = faceNormal;
                tri[2].normal = faceNormal;
            }

            vertices.push_back(tri[0]);
            vertices.push_back(tri[1]);
            vertices.push_back(tri[2]);
            indexOffset += fv;
        }
    }

    m_vertexCount = static_cast<int>(vertices.size());
    if (m_vertexCount == 0)
    {
        return false;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(vertices.size() * sizeof(SkeletonVertex)),
        vertices.data(),
        GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletonVertex), reinterpret_cast<void*>(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SkeletonVertex),
        reinterpret_cast<void*>(offsetof(SkeletonVertex, normal)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SkeletonVertex),
        reinterpret_cast<void*>(offsetof(SkeletonVertex, regionId)));

    glBindVertexArray(0);

    m_isLoaded = true;
    setupRegionBounds();

    for (int i = 0; i < 3; ++i)
    {
        if (hasRegBounds[i])
        {
            m_regions[i].minBounds = regMin[i];
            m_regions[i].maxBounds = regMax[i];
        }
    }

    return true;
}

void SkeletonExplorerScreen::destroySkeletonBuffers()
{
    if (m_vbo != 0)
    {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao != 0)
    {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    m_vertexCount = 0;
    m_isLoaded = false;
}

void SkeletonExplorerScreen::setupRegionBounds()
{
    m_regions[0] = {
        SpineRegionId::Cervical,
        "Cervical Spine",
        "C1 - C7 (7 Ruas Leher)",
        "Bagian paling atas tulang belakang di area leher. Mendukung berat kepala, "
        "melindungi sumsum tulang belakang atas, dan memungkinkan gerakan rotasi serta "
        "anggukan kepala dengan kelengkungan alami lordosis.",
        glm::vec3(m_spineCenterXZ.x - m_spineRadiusXZ, m_cervicalMinY, m_spineCenterXZ.y - m_spineRadiusXZ),
        glm::vec3(m_spineCenterXZ.x + m_spineRadiusXZ, m_cervicalMaxY, m_spineCenterXZ.y + m_spineRadiusXZ) };

    m_regions[1] = {
        SpineRegionId::Thoracic,
        "Thoracic Spine",
        "T1 - T12 (12 Ruas Punggung Atas)",
        "Segmen terpanjang di bagian punggung atas hingga tengah yang terhubung langsung "
        "dengan tulang rusuk (rib cage). Regio ini paling sering menjadi lokasi kurva utama "
        "pada kasus skoliosis struktural.",
        glm::vec3(m_spineCenterXZ.x - m_spineRadiusXZ, m_thoracicMinY, m_spineCenterXZ.y - m_spineRadiusXZ),
        glm::vec3(m_spineCenterXZ.x + m_spineRadiusXZ, m_thoracicMaxY, m_spineCenterXZ.y + m_spineRadiusXZ) };

    m_regions[2] = {
        SpineRegionId::Lumbar,
        "Lumbar Spine",
        "L1 - L5 (5 Ruas Punggung Bawah)",
        "Terletak di punggung bagian bawah di atas panggul (sacrum). Memiliki ukuran ruas "
        "paling besar dan tebal karena menopang sebagian besar beban tubuh bagian atas.",
        glm::vec3(m_spineCenterXZ.x - m_spineRadiusXZ, m_lumbarMinY, m_spineCenterXZ.y - m_spineRadiusXZ),
        glm::vec3(m_spineCenterXZ.x + m_spineRadiusXZ, m_lumbarMaxY, m_spineCenterXZ.y + m_spineRadiusXZ) };
}

SpineRegionId SkeletonExplorerScreen::pickRegion(double mouseX, double mouseY,
    int viewportW, int viewportH) const
{
    if (viewportW <= 0 || viewportH <= 0)
    {
        return SpineRegionId::None;
    }

    const glm::mat4 view = m_camera.viewMatrix();
    const glm::mat4 proj = m_camera.projectionMatrix();
    const glm::vec3 rayOriginWorld = m_camera.position();
    const glm::vec3 rayDirWorld = screenToWorldRay(mouseX, mouseY, viewportW, viewportH, view, proj);

    const glm::mat4 invModel = glm::inverse(m_modelMatrix);
    const glm::vec3 rayOrigLocal = glm::vec3(invModel * glm::vec4(rayOriginWorld, 1.0f));
    const glm::vec3 rayDirLocal = glm::normalize(glm::vec3(invModel * glm::vec4(rayDirWorld, 0.0f)));

    float closestT = std::numeric_limits<float>::max();
    SpineRegionId hitRegion = SpineRegionId::None;

    for (const auto& region : m_regions)
    {
        float tHit = 0.0f;
        if (intersectRayAABB(rayOrigLocal, rayDirLocal, region.minBounds, region.maxBounds, tHit))
        {
            if (tHit < closestT)
            {
                closestT = tHit;
                hitRegion = region.id;
            }
        }
    }

    return hitRegion;
}

glm::vec3 SkeletonExplorerScreen::screenToWorldRay(double mouseX, double mouseY, int screenW, int screenH,
    const glm::mat4& view, const glm::mat4& proj)
{
    const float ndcX = (2.0f * static_cast<float>(mouseX)) / static_cast<float>(screenW) - 1.0f;
    const float ndcY = 1.0f - (2.0f * static_cast<float>(mouseY)) / static_cast<float>(screenH);

    const glm::vec4 rayClip(ndcX, ndcY, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(proj) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    const glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    return glm::normalize(rayWorld);
}

bool SkeletonExplorerScreen::intersectRayAABB(const glm::vec3& rayOrig, const glm::vec3& rayDir,
    const glm::vec3& boxMin, const glm::vec3& boxMax,
    float& tHit)
{
    const glm::vec3 safeDir(
        std::abs(rayDir.x) < 1e-6f ? (rayDir.x >= 0.0f ? 1e-6f : -1e-6f) : rayDir.x,
        std::abs(rayDir.y) < 1e-6f ? (rayDir.y >= 0.0f ? 1e-6f : -1e-6f) : rayDir.y,
        std::abs(rayDir.z) < 1e-6f ? (rayDir.z >= 0.0f ? 1e-6f : -1e-6f) : rayDir.z);

    const glm::vec3 invDir = 1.0f / safeDir;
    const glm::vec3 t0 = (boxMin - rayOrig) * invDir;
    const glm::vec3 t1 = (boxMax - rayOrig) * invDir;

    const glm::vec3 tMin = glm::min(t0, t1);
    const glm::vec3 tMax = glm::max(t0, t1);

    const float tNear = std::max({ tMin.x, tMin.y, tMin.z });
    const float tFar = std::min({ tMax.x, tMax.y, tMax.z });

    if (tNear > tFar || tFar < 0.0f)
    {
        return false;
    }

    tHit = (tNear > 0.0f) ? tNear : tFar;
    return true;
}