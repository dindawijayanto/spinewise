#include "screens/MenuScreen.h"

#include "core/App.h"

#include <imgui.h>

void MenuScreen::drawUI()
{
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f), ImGuiCond_Always);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("SPINEWISE", nullptr, flags))
    {
        ImGui::TextWrapped("Aplikasi edukasi 3D interaktif tentang anatomi tulang belakang dan skoliosis.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        const ImVec2 buttonSize(-1.0f, 40.0f);
        if (ImGui::Button("Skeleton Explorer", buttonSize))
            m_app.switchTo(ScreenId::SkeletonExplorer);
        if (ImGui::Button("Spine Lab", buttonSize))
            m_app.switchTo(ScreenId::SpineLab);
        if (ImGui::Button("Posture Lab", buttonSize))
            m_app.switchTo(ScreenId::PostureLab);

        ImGui::Spacing();
        if (ImGui::Button("Keluar", buttonSize))
            m_app.quit();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::TextDisabled("Aplikasi edukasi, bukan alat diagnosis.");
        // TODO(Dinda): panel About & Credits (isi dari Rafiq)
    }
    ImGui::End();
}
