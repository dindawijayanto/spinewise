#include "screens/PostureLabScreen.h"

#include "core/App.h"
#include "render/Shader.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>
#include <glm/gtc/constants.hpp>
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
    constexpr float kSidePanelRatio = 0.34f;

    float smoothStep01(float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

    float lerpFloat(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    void resetPostureCamera(OrbitCamera& camera, OrbitCameraInput& input, glm::mat4& modelMatrix)
    {
        camera.reset();
        input.reset();

        // Hadapkan belakang (tampak punggung/spine, bahu, dan pelvis) + sedikit zoom
        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(1.18f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        modelMatrix = model;
    }

    // Evaluasi pergeseran lateral X dan sudut miring (radian di bidang frontal X-Z Blender)
    // u in [0, 1] dari dasar lumbar (L5) ke puncak thoracic (T1)
    void evaluateSpineCurve(const Spinewise::SpineParams& params, float u, float& outOffsetX, float& outTiltRad)
    {
        const float dirSign = (params.primaryDirection == Spinewise::Direction::Right) ? -1.0f : 1.0f;
        const float primaryAmp = (params.primaryCobb / 70.0f) * 0.28f;

        if (params.pattern == Spinewise::CurvePattern::C)
        {
            float centerU = 0.62f;
            float width = 0.48f;
            if (params.primaryRegion == Spinewise::Region::Thoracolumbar)
            {
                centerU = 0.46f;
                width = 0.50f;
            }
            else if (params.primaryRegion == Spinewise::Region::Lumbar)
            {
                centerU = 0.28f;
                width = 0.44f;
            }

            auto evalX = [&](float t) -> float {
                const float arch = std::sin(glm::pi<float>() * t);
                const float focus = std::exp(-std::pow((t - centerU) / width, 2.0f));
                return dirSign * primaryAmp * arch * (0.55f + 0.65f * focus);
                };

            outOffsetX = evalX(u);

            const float eps = 0.01f;
            const float u0 = std::clamp(u - eps, 0.0f, 1.0f);
            const float u1 = std::clamp(u + eps, 0.0f, 1.0f);
            const float dxdu = (evalX(u1) - evalX(u0)) / std::max(0.0001f, (u1 - u0));
            outTiltRad = -std::atan(dxdu * 0.65f);
        }
        else
        {
            const float secondaryAmp = (params.secondaryCobb / 70.0f) * 0.22f;

            auto evalX = [&](float t) -> float {
                const float upperWave = std::sin(glm::pi<float>() * std::clamp((t - 0.36f) / 0.64f, 0.0f, 1.0f));
                const float lowerWave = std::sin(glm::pi<float>() * std::clamp(t / 0.54f, 0.0f, 1.0f));
                return dirSign * (primaryAmp * upperWave - secondaryAmp * lowerWave);
                };

            outOffsetX = evalX(u);

            const float eps = 0.01f;
            const float u0 = std::clamp(u - eps, 0.0f, 1.0f);
            const float u1 = std::clamp(u + eps, 0.0f, 1.0f);
            const float dxdu = (evalX(u1) - evalX(u0)) / std::max(0.0001f, (u1 - u0));
            outTiltRad = -std::atan(dxdu * 0.65f);
        }
    }

    // Rotasi 2D di bidang frontal (X, Z) terhadap titik pivot (pivotX, pivotZ)
    glm::vec3 rotateFrontalXZ(const glm::vec3& p, float pivotX, float pivotZ, float angleRad)
    {
        const float dx = p.x - pivotX;
        const float dz = p.z - pivotZ;
        const float c = std::cos(angleRad);
        const float s = std::sin(angleRad);
        return glm::vec3(
            pivotX + dx * c - dz * s,
            p.y,
            pivotZ + dx * s + dz * c);
    }

    glm::vec3 rotateNormalXZ(const glm::vec3& n, float angleRad)
    {
        const float c = std::cos(angleRad);
        const float s = std::sin(angleRad);
        return glm::normalize(glm::vec3(
            n.x * c - n.z * s,
            n.y,
            n.x * s + n.z * c));
    }
} // namespace

PostureLabScreen::PostureLabScreen(App& app)
    : Screen(app)
{
    loadPreset(AnimationPreset::CCurveProgression);
}

PostureLabScreen::~PostureLabScreen()
{
    destroySkeletonBuffers();
}

void PostureLabScreen::onEnter()
{
    m_loadError.clear();

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
        if (!loadSkeletonBase("assets/models/Skeletons.obj"))
        {
            if (m_loadError.empty())
            {
                m_loadError = "Gagal memuat assets/models/Skeletons.obj (pastikan Git LFS sudah di-pull).";
            }
            return;
        }
    }

    resetPostureCamera(m_camera, m_cameraInput, m_modelMatrix);
    m_currentTime = 0.0f;
    m_isPlaying = true;
    m_lastEvaluatedPrimary = -1.0f;
}

void PostureLabScreen::onExit()
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

void PostureLabScreen::update(float dt)
{
    GLFWwindow* win = m_app.window();
    if (!win)
    {
        return;
    }

    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(win, &fbW, &fbH);
    if (fbW > 0 && fbH > 0)
    {
        const int panelFbW = static_cast<int>(static_cast<float>(fbW) * kSidePanelRatio);
        const int viewFbW = std::max(1, fbW - panelFbW);
        m_camera.setAspectRatio(static_cast<float>(viewFbW) / static_cast<float>(fbH));
    }

    m_cameraInput.update(win, m_camera);
    m_camera.update(dt);

    if (m_isPlaying && m_duration > 0.0f)
    {
        m_currentTime += dt * m_playbackSpeed;
        if (m_currentTime >= m_duration)
        {
            if (m_loop)
            {
                m_currentTime = std::fmod(m_currentTime, m_duration);
            }
            else
            {
                m_currentTime = m_duration;
                m_isPlaying = false;
            }
        }
    }

    const float normT = (m_duration > 0.0f) ? std::clamp(m_currentTime / m_duration, 0.0f, 1.0f) : 0.0f;
    m_activeParams = evaluateKeyframes(normT);
    m_activeParams.clampValues();

    const int patInt = static_cast<int>(m_activeParams.pattern);
    if (m_isLoaded &&
        (std::abs(m_activeParams.primaryCobb - m_lastEvaluatedPrimary) > 0.01f ||
            std::abs(m_activeParams.secondaryCobb - m_lastEvaluatedSecondary) > 0.01f ||
            patInt != m_lastEvaluatedPattern))
    {
        updateDeformedMesh(m_activeParams);
        m_lastEvaluatedPrimary = m_activeParams.primaryCobb;
        m_lastEvaluatedSecondary = m_activeParams.secondaryCobb;
        m_lastEvaluatedPattern = patInt;
    }
}

void PostureLabScreen::render()
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
    m_shader->setInt("uHoveredRegion", 0);
    m_shader->setInt("uSelectedRegion", 0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<int>(m_deformedVertices.size()));
    glBindVertexArray(0);

    glViewport(0, 0, fbW, fbH);
}

void PostureLabScreen::drawUI()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float panelW = vp->Size.x * kSidePanelRatio;

    ImGui::SetNextWindowPos(vp->Pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelW, vp->Size.y), ImGuiCond_Always);

    const ImGuiWindowFlags panelFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("Posture Lab", nullptr, panelFlags);

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

    // Label wajib sesuai spesifikasi tugas: konsep postur, bukan latihan
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.28f, 1.0f));
    ImGui::TextWrapped("Label Edukasi: Visualisasi konsep biomekanika postur (bukan panduan latihan atau terapi medis).");
    ImGui::PopStyleColor();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 1. Pilihan Preset Animasi Keyframe
    ImGui::Text("Skenario Animasi Keyframe:");
    const char* presetNames[] = {
        "1. Progresi Kurva C (Thoracic Kanan)",
        "2. Kompensasi Kurva S (Bahu vs Pelvis)",
        "3. Siklus Keselarasan Postur"
    };
    int presetIdx = static_cast<int>(m_currentPreset);
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::Combo("##preset", &presetIdx, presetNames, IM_ARRAYSIZE(presetNames)))
    {
        loadPreset(static_cast<AnimationPreset>(presetIdx));
        m_currentTime = 0.0f;
        m_isPlaying = true;
        m_lastEvaluatedPrimary = -1.0f;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 2. Kontrol Playback: Play / Pause / Replay + Timeline
    ImGui::Text("Kontrol Animasi:");
    const float btnW = (ImGui::GetContentRegionAvail().x - 8.0f) * 0.5f;

    if (ImGui::Button(m_isPlaying ? "Pause" : "Play", ImVec2(btnW, 32.0f)))
    {
        if (!m_isPlaying && m_currentTime >= m_duration)
        {
            m_currentTime = 0.0f;
        }
        m_isPlaying = !m_isPlaying;
    }
    ImGui::SameLine();
    if (ImGui::Button("Replay", ImVec2(btnW, 32.0f)))
    {
        m_currentTime = 0.0f;
        m_isPlaying = true;
    }

    ImGui::Spacing();
    ImGui::Text("Timeline (%.2fs / %.2fs):", m_currentTime, m_duration);
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::SliderFloat("##timeline", &m_currentTime, 0.0f, m_duration, "%.2f s"))
    {
        m_isPlaying = false;
    }

    if (!m_keyframes.empty())
    {
        ImGui::TextDisabled("Loncat ke Keyframe:");
        for (size_t i = 0; i < m_keyframes.size(); ++i)
        {
            if (i > 0) ImGui::SameLine();
            char kfBtn[32];
            std::snprintf(kfBtn, sizeof(kfBtn), "KF %zu", i + 1);
            if (ImGui::SmallButton(kfBtn))
            {
                m_currentTime = m_keyframes[i].time * m_duration;
                m_isPlaying = false;
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("%s (%.0f%%)", m_keyframes[i].label.c_str(), m_keyframes[i].time * 100.0f);
            }
        }
    }

    ImGui::Spacing();
    ImGui::SetNextItemWidth(-1.0f);
    ImGui::SliderFloat("Kecepatan", &m_playbackSpeed, 0.25f, 2.5f, "%.2fx");
    ImGui::Checkbox("Loop Animasi", &m_loop);
    if (ImGui::Checkbox("Tampilkan Lengan & Tungkai", &m_showLimbs))
    {
        m_lastEvaluatedPrimary = -1.0f;
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // 3. Ringkasan Biomekanika Real-time
    ImGui::Text("Respon Biomekanika Kerangka:");
    ImGui::BulletText("Pola Kurva: %s", m_activeParams.pattern == Spinewise::CurvePattern::C ? "Kurva C" : "Kurva S");
    ImGui::BulletText("Sudut Cobb Utama: %.1f deg", m_activeParams.primaryCobb);
    if (m_activeParams.pattern == Spinewise::CurvePattern::S)
    {
        ImGui::BulletText("Sudut Cobb Sekunder: %.1f deg", m_activeParams.secondaryCobb);
    }
    ImGui::BulletText("Kemiringan Bahu (Thorax): %.1f deg", m_shoulderTiltDeg);
    ImGui::BulletText("Kemiringan Pelvis: %.1f deg", m_pelvisTiltDeg);

    ImGui::Spacing();
    ImGui::TextWrapped("Bahu (thorax & clavicula) miring mengikuti kurva thoracic atas, sedangkan pelvis merespons lengkungan lumbar bawah untuk menjaga keseimbangan tubuh.");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Reset View", ImVec2(-1.0f, 30.0f)))
    {
        resetPostureCamera(m_camera, m_cameraInput, m_modelMatrix);
    }

    ImGui::Spacing();
    if (ImGui::Button("Kembali ke Menu", ImVec2(-1.0f, 32.0f)))
    {
        m_app.switchTo(ScreenId::Menu);
    }

    ImGui::End();
}

void PostureLabScreen::loadPreset(AnimationPreset preset)
{
    m_currentPreset = preset;
    m_keyframes.clear();

    if (preset == AnimationPreset::CCurveProgression)
    {
        m_duration = 5.0f;

        Spinewise::SpineParams kf0{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 0.0f, 0.0f };
        Spinewise::SpineParams kf1{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 18.0f, 0.0f };
        Spinewise::SpineParams kf2{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 38.0f, 0.0f };
        Spinewise::SpineParams kf3{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 55.0f, 0.0f };

        m_keyframes.push_back({ 0.00f, kf0, "Postur Netral (0 deg)" });
        m_keyframes.push_back({ 0.35f, kf1, "Asimetri Ringan (18 deg)" });
        m_keyframes.push_back({ 0.70f, kf2, "Kurva C Sedang (38 deg)" });
        m_keyframes.push_back({ 1.00f, kf3, "Kurva C Signifikan (55 deg)" });
    }
    else if (preset == AnimationPreset::SCurveCompensation)
    {
        m_duration = 6.0f;

        Spinewise::SpineParams kf0{ Spinewise::CurvePattern::S, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 0.0f, 0.0f };
        Spinewise::SpineParams kf1{ Spinewise::CurvePattern::S, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 28.0f, 14.0f };
        Spinewise::SpineParams kf2{ Spinewise::CurvePattern::S, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 48.0f, 40.0f };
        Spinewise::SpineParams kf3{ Spinewise::CurvePattern::S, Spinewise::Region::Thoracic, Spinewise::Direction::Right, 42.0f, 42.0f };

        m_keyframes.push_back({ 0.00f, kf0, "Netral Sejajar" });
        m_keyframes.push_back({ 0.33f, kf1, "Kurva Thoracic Muncul" });
        m_keyframes.push_back({ 0.70f, kf2, "Kompensasi Lumbar & Pelvis" });
        m_keyframes.push_back({ 1.00f, kf3, "Keseimbangan Kompensatoris" });
    }
    else
    {
        m_duration = 5.5f;

        Spinewise::SpineParams kf0{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracolumbar, Spinewise::Direction::Left, 0.0f, 0.0f };
        Spinewise::SpineParams kf1{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracolumbar, Spinewise::Direction::Left, 44.0f, 0.0f };
        Spinewise::SpineParams kf2{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracolumbar, Spinewise::Direction::Left, 15.0f, 0.0f };
        Spinewise::SpineParams kf3{ Spinewise::CurvePattern::C, Spinewise::Region::Thoracolumbar, Spinewise::Direction::Left, 0.0f, 0.0f };

        m_keyframes.push_back({ 0.00f, kf0, "Awal: Sejajar" });
        m_keyframes.push_back({ 0.45f, kf1, "Puncak Deviasi Postur" });
        m_keyframes.push_back({ 0.78f, kf2, "Transisi Keselarasan" });
        m_keyframes.push_back({ 1.00f, kf3, "Kembali Sejajar" });
    }
}

Spinewise::SpineParams PostureLabScreen::evaluateKeyframes(float normalizedTime) const
{
    if (m_keyframes.empty())
    {
        return Spinewise::SpineParams{};
    }
    if (normalizedTime <= m_keyframes.front().time)
    {
        return m_keyframes.front().params;
    }
    if (normalizedTime >= m_keyframes.back().time)
    {
        return m_keyframes.back().params;
    }

    for (size_t i = 0; i + 1 < m_keyframes.size(); ++i)
    {
        const auto& k0 = m_keyframes[i];
        const auto& k1 = m_keyframes[i + 1];
        if (normalizedTime >= k0.time && normalizedTime <= k1.time)
        {
            const float span = std::max(0.0001f, k1.time - k0.time);
            const float localT = smoothStep01((normalizedTime - k0.time) / span);

            Spinewise::SpineParams out = k0.params;
            out.primaryCobb = lerpFloat(k0.params.primaryCobb, k1.params.primaryCobb, localT);
            out.secondaryCobb = lerpFloat(k0.params.secondaryCobb, k1.params.secondaryCobb, localT);
            return out;
        }
    }

    return m_keyframes.back().params;
}

bool PostureLabScreen::loadSkeletonBase(const std::string& objPath)
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

    float lumbarMinZ = std::numeric_limits<float>::max();
    float thoracicMaxZ = std::numeric_limits<float>::lowest();
    float pelvisMinZ = std::numeric_limits<float>::max();
    float pelvisMaxZ = std::numeric_limits<float>::lowest();
    float thoraxMaxZ = std::numeric_limits<float>::lowest();
    float feetMinZ = std::numeric_limits<float>::max();

    m_baseVertices.clear();

    for (const auto& shape : shapes)
    {
        std::string lower = shape.name;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        AnatomyPart part = AnatomyPart::Other;
        float regionId = 0.0f;

        if (lower.find("cervical") != std::string::npos)
        {
            part = AnatomyPart::Cervical;
            regionId = 1.0f;
        }
        else if (lower.find("thoracic") != std::string::npos)
        {
            part = AnatomyPart::Thoracic;
            regionId = 2.0f;
        }
        else if (lower.find("lumbar") != std::string::npos)
        {
            part = AnatomyPart::Lumbar;
            regionId = 3.0f;
        }
        else if (lower.find("pelvis") != std::string::npos)
        {
            part = AnatomyPart::Pelvis;
        }
        else if (lower.find("sacrum") != std::string::npos)
        {
            part = AnatomyPart::Sacrum;
        }
        else if (lower.find("skull") != std::string::npos)
        {
            part = AnatomyPart::Skull;
        }
        else if (lower.find("thorax") != std::string::npos)
        {
            part = AnatomyPart::ThoraxShoulder;
        }
        else if (lower.find("upper_limb") != std::string::npos)
        {
            part = AnatomyPart::UpperLimb;
        }
        else if (lower.find("lower_limb") != std::string::npos)
        {
            part = AnatomyPart::LowerLimb;
        }

        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f)
        {
            const size_t fv = static_cast<size_t>(shape.mesh.num_face_vertices[f]);
            if (fv != 3)
            {
                indexOffset += fv;
                continue;
            }

            BaseVertexData tri[3];
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
                tri[v].regionId = regionId;
                tri[v].part = part;

                if (part == AnatomyPart::Lumbar) lumbarMinZ = std::min(lumbarMinZ, tri[v].pos.z);
                if (part == AnatomyPart::Thoracic) thoracicMaxZ = std::max(thoracicMaxZ, tri[v].pos.z);
                if (part == AnatomyPart::Pelvis)
                {
                    pelvisMinZ = std::min(pelvisMinZ, tri[v].pos.z);
                    pelvisMaxZ = std::max(pelvisMaxZ, tri[v].pos.z);
                }
                if (part == AnatomyPart::ThoraxShoulder) thoraxMaxZ = std::max(thoraxMaxZ, tri[v].pos.z);
                if (part == AnatomyPart::LowerLimb) feetMinZ = std::min(feetMinZ, tri[v].pos.z);

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
                    : glm::vec3(0.0f, 0.0f, 1.0f);
                tri[0].normal = faceNormal;
                tri[1].normal = faceNormal;
                tri[2].normal = faceNormal;
            }

            m_baseVertices.push_back(tri[0]);
            m_baseVertices.push_back(tri[1]);
            m_baseVertices.push_back(tri[2]);
            indexOffset += fv;
        }
    }

    if (m_baseVertices.empty())
    {
        return false;
    }

    m_spineMinZ = (lumbarMinZ < thoracicMaxZ) ? lumbarMinZ : -0.1f;
    m_spineMaxZ = (thoracicMaxZ > m_spineMinZ) ? thoracicMaxZ : 0.7f;
    m_pelvisPivotZ = (pelvisMinZ < pelvisMaxZ) ? (0.5f * (pelvisMinZ + pelvisMaxZ)) : (m_spineMinZ - 0.1f);
    m_shoulderPivotZ = (thoraxMaxZ > m_spineMinZ) ? (thoraxMaxZ - 0.06f) : m_spineMaxZ;
    m_feetMinZ = (feetMinZ < m_pelvisPivotZ) ? feetMinZ : -1.7f;

    m_deformedVertices.resize(m_baseVertices.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr>(m_deformedVertices.size() * sizeof(SkeletonVertex)),
        nullptr,
        GL_DYNAMIC_DRAW);

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
    updateDeformedMesh(m_activeParams);
    return true;
}

void PostureLabScreen::destroySkeletonBuffers()
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
    m_baseVertices.clear();
    m_deformedVertices.clear();
    m_isLoaded = false;
}

void PostureLabScreen::updateDeformedMesh(const Spinewise::SpineParams& params)
{
    if (!m_isLoaded || m_baseVertices.empty())
    {
        return;
    }

    const float spineSpanZ = std::max(0.001f, m_spineMaxZ - m_spineMinZ);

    // Evaluasi kemiringan & pergeseran di dasar lumbar (Pelvis) dan puncak thoracic (Bahu)
    float pelvisOffsetX = 0.0f, pelvisRawTilt = 0.0f;
    float shoulderOffsetX = 0.0f, shoulderRawTilt = 0.0f;
    evaluateSpineCurve(params, 0.04f, pelvisOffsetX, pelvisRawTilt);
    evaluateSpineCurve(params, 0.96f, shoulderOffsetX, shoulderRawTilt);

    const float pelvisTiltRad = pelvisRawTilt * 0.65f;
    const float shoulderTiltRad = shoulderRawTilt * 1.15f;

    m_pelvisTiltDeg = glm::degrees(pelvisTiltRad);
    m_shoulderTiltDeg = glm::degrees(shoulderTiltRad);

    for (size_t i = 0; i < m_baseVertices.size(); ++i)
    {
        const auto& src = m_baseVertices[i];
        glm::vec3 pos = src.pos;
        glm::vec3 norm = src.normal;

        switch (src.part)
        {
        case AnatomyPart::Lumbar:
        case AnatomyPart::Thoracic:
        {
            const float u = std::clamp((pos.z - m_spineMinZ) / spineSpanZ, 0.0f, 1.0f);
            float curveX = 0.0f, curveTilt = 0.0f;
            evaluateSpineCurve(params, u, curveX, curveTilt);

            // Rotasi tiap vertebra terhadap sumbu pusat tulang belakang di ketinggian z-nya
            pos = rotateFrontalXZ(pos, 0.0f, pos.z, curveTilt);
            pos.x += curveX;
            norm = rotateNormalXZ(norm, curveTilt);
            break;
        }

        case AnatomyPart::Pelvis:
        case AnatomyPart::Sacrum:
        {
            pos = rotateFrontalXZ(pos, 0.0f, m_pelvisPivotZ, pelvisTiltRad);
            pos.x += pelvisOffsetX;
            norm = rotateNormalXZ(norm, pelvisTiltRad);
            break;
        }

        case AnatomyPart::ThoraxShoulder:
        {
            // Bagian atas sangkar dada & bahu (clavicula/scapula) mengikuti kemiringan bahu
            // Bagian bawah sangkar dada bertransisi halus menuju kurva thoracic tengah
            const float u = std::clamp((pos.z - m_spineMinZ) / spineSpanZ, 0.0f, 1.0f);
            float curveX = 0.0f, curveTilt = 0.0f;
            evaluateSpineCurve(params, u, curveX, curveTilt);

            const float blendShoulder = smoothStep01((u - 0.35f) / 0.65f);
            const float effectiveTilt = lerpFloat(curveTilt * 0.7f, shoulderTiltRad, blendShoulder);
            const float effectiveShiftX = lerpFloat(curveX, shoulderOffsetX, blendShoulder);
            const float pivotZ = lerpFloat(pos.z, m_shoulderPivotZ, blendShoulder);

            pos = rotateFrontalXZ(pos, 0.0f, pivotZ, effectiveTilt);
            pos.x += effectiveShiftX;
            norm = rotateNormalXZ(norm, effectiveTilt);
            break;
        }

        case AnatomyPart::UpperLimb:
        {
            if (!m_showLimbs)
            {
                pos = glm::vec3(0.0f);
                break;
            }
            // Lengan ikut bergeser & miring bersama sendi bahu
            pos = rotateFrontalXZ(pos, 0.0f, m_shoulderPivotZ, shoulderTiltRad * 0.75f);
            pos.x += shoulderOffsetX;
            norm = rotateNormalXZ(norm, shoulderTiltRad * 0.75f);
            break;
        }

        case AnatomyPart::Cervical:
        {
            const float tNeck = std::clamp((pos.z - m_spineMaxZ) / 0.32f, 0.0f, 1.0f);
            const float neckTilt = shoulderTiltRad * (1.0f - 0.65f * tNeck);
            const float neckShiftX = shoulderOffsetX * (1.0f - 0.25f * tNeck);

            pos = rotateFrontalXZ(pos, 0.0f, m_spineMaxZ, neckTilt);
            pos.x += neckShiftX;
            norm = rotateNormalXZ(norm, neckTilt);
            break;
        }

        case AnatomyPart::Skull:
        {
            // Kepala tetap relatif tegak (righting reflex) mengikuti ujung leher
            const float headTilt = shoulderTiltRad * 0.20f;
            const float headShiftX = shoulderOffsetX * 0.72f;
            const float skullPivotZ = m_spineMaxZ + 0.30f;

            pos = rotateFrontalXZ(pos, 0.0f, skullPivotZ, headTilt);
            pos.x += headShiftX;
            norm = rotateNormalXZ(norm, headTilt);
            break;
        }

        case AnatomyPart::LowerLimb:
        {
            if (!m_showLimbs)
            {
                pos = glm::vec3(0.0f);
                break;
            }
            // Sendi panggul atas mengikuti pelvis, telapak kaki tetap menapak di lantai
            const float legSpan = std::max(0.001f, m_pelvisPivotZ - m_feetMinZ);
            const float hipWeight = smoothStep01((pos.z - m_feetMinZ) / legSpan);
            pos.x += pelvisOffsetX * hipWeight;
            break;
        }

        default:
            break;
        }

        m_deformedVertices[i].pos = pos;
        m_deformedVertices[i].normal = norm;
        m_deformedVertices[i].regionId = src.regionId;
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
        static_cast<GLsizeiptr>(m_deformedVertices.size() * sizeof(SkeletonVertex)),
        m_deformedVertices.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}