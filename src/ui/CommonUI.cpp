#include "ui/CommonUI.h"

#include "core/App.h"

#include <imgui.h>

namespace ui
{
void drawTopBar(App& app, const char* screenName)
{
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.6f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    if (ImGui::Begin("##TopBar", nullptr, flags))
    {
        if (ImGui::Button("< Menu"))
            app.switchTo(ScreenId::Menu);
        ImGui::SameLine();
        ImGui::TextUnformatted(screenName);
        ImGui::SameLine();
        ImGui::TextDisabled("| %.0f FPS", ImGui::GetIO().Framerate);
    }
    ImGui::End();
}
} // namespace ui
