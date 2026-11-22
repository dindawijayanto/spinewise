#include "ui/SpineParamsPanel.h"

#include <imgui.h>

namespace
{
using namespace Spinewise;

constexpr float kMinCobb = 0.0f;
constexpr float kMaxCobb = 70.0f;
constexpr float kScoliosisThreshold = 10.0f;

const ImVec4 kWarningColor(1.0f, 0.75f, 0.30f, 1.0f);
const ImVec4 kInfoColor(0.55f, 0.80f, 1.0f, 1.0f);

const char* regionLabel(Region region)
{
    switch (region)
    {
    case Region::Thoracic: return "Thoracic";
    case Region::Thoracolumbar: return "Thoracolumbar";
    case Region::Lumbar: return "Lumbar";
    }
    return "-";
}

const char* directionLabel(Direction direction)
{
    return direction == Direction::Left ? "Kiri" : "Kanan";
}

// Kategori umum berdasarkan besar Cobb angle (hanya untuk konteks edukasi).
const char* severityLabel(float cobb)
{
    if (cobb < kScoliosisThreshold) return "Di bawah ambang skoliosis";
    if (cobb < 25.0f) return "Ringan";
    if (cobb < 40.0f) return "Sedang";
    return "Berat";
}

template <typename Enum>
bool radioEnum(const char* label, Enum& value, Enum option)
{
    if (ImGui::RadioButton(label, value == option))
    {
        if (value != option)
        {
            value = option;
            return true;
        }
    }
    return false;
}

bool cobbSlider(const char* label, float& cobb)
{
    const bool changed = ImGui::SliderFloat(label, &cobb, kMinCobb, kMaxCobb, "%.0f deg", ImGuiSliderFlags_AlwaysClamp);
    ImGui::TextDisabled("Kategori: %s", severityLabel(cobb));
    return changed;
}
} // namespace

namespace ui
{
bool drawSpineParamsPanel(SpineParams& params)
{
    bool changed = false;

    const ImGuiIO& io = ImGui::GetIO();
    const float panelWidth = 320.0f;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - panelWidth - 10.0f, 10.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.85f);
    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin("Parameter Kurva", nullptr, flags))
    {
        ImGui::End();
        return false;
    }

    ImGui::PushItemWidth(-1.0f);

    // Pola kurva
    ImGui::SeparatorText("Pola Kurva");
    changed |= radioEnum("C-shaped", params.pattern, CurvePattern::C);
    ImGui::SameLine();
    changed |= radioEnum("S-shaped", params.pattern, CurvePattern::S);

    // Kurva primer
    ImGui::SeparatorText("Kurva Primer");
    ImGui::TextUnformatted("Region");
    changed |= radioEnum("Thoracic", params.primaryRegion, Region::Thoracic);
    ImGui::SameLine();
    changed |= radioEnum("Thoracolumbar", params.primaryRegion, Region::Thoracolumbar);
    ImGui::SameLine();
    changed |= radioEnum("Lumbar", params.primaryRegion, Region::Lumbar);

    ImGui::TextUnformatted("Arah (sisi konveks)");
    changed |= radioEnum("Kiri##primary", params.primaryDirection, Direction::Left);
    ImGui::SameLine();
    changed |= radioEnum("Kanan##primary", params.primaryDirection, Direction::Right);

    ImGui::TextUnformatted("Cobb angle");
    changed |= cobbSlider("##primaryCobb", params.primaryCobb);

    // Kurva sekunder: hanya muncul untuk S-shaped
    if (params.pattern == CurvePattern::S)
    {
        ImGui::SeparatorText("Kurva Sekunder");
        const Direction secondaryDirection =
            params.primaryDirection == Direction::Left ? Direction::Right : Direction::Left;
        ImGui::TextColored(kInfoColor, "Arah otomatis berlawanan: %s", directionLabel(secondaryDirection));

        ImGui::TextUnformatted("Cobb angle");
        changed |= cobbSlider("##secondaryCobb", params.secondaryCobb);
    }

    ImGui::PopItemWidth();

    // Validasi
    params.clampValues();

    ImGui::Separator();
    bool hasWarning = false;
    if (params.primaryCobb < kScoliosisThreshold)
    {
        ImGui::TextColored(kWarningColor, "Cobb primer < %.0f deg: belum termasuk skoliosis.", kScoliosisThreshold);
        hasWarning = true;
    }
    if (params.pattern == CurvePattern::S && params.secondaryCobb < kScoliosisThreshold)
    {
        ImGui::TextColored(kWarningColor, "S-shaped butuh Cobb sekunder >= %.0f deg.", kScoliosisThreshold);
        hasWarning = true;
    }
    if (!hasWarning)
    {
        ImGui::TextColored(kInfoColor, "%s, %s %s, %.0f deg",
                           params.pattern == CurvePattern::C ? "C-shaped" : "S-shaped",
                           regionLabel(params.primaryRegion), directionLabel(params.primaryDirection),
                           params.primaryCobb);
    }

    if (ImGui::Button("Reset ke Normal", ImVec2(-1.0f, 0.0f)))
    {
        params = SpineParams{};
        changed = true;
    }

    // Disclaimer
    ImGui::Spacing();
    ImGui::SeparatorText("Disclaimer");
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextDisabled("SPINEWISE adalah alat visualisasi edukasi. Aplikasi ini tidak mendiagnosis skoliosis, "
                        "tidak membaca hasil X-ray, dan tidak meresepkan latihan. Konsultasikan kondisi Anda "
                        "dengan tenaga medis.");
    ImGui::PopTextWrapPos();

    ImGui::End();
    return changed;
}
} // namespace ui
