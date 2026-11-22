#pragma once

#include "spine/SpineParams.h"

// Owner: Dinda
// Panel input parameter skoliosis (ImGui) untuk Spine Lab.
namespace ui
{
// Menggambar panel input + validasi + disclaimer, menulis langsung ke params.
// Return true kalau ada nilai yang berubah di frame ini (dipakai untuk memicu deformasi ulang).
bool drawSpineParamsPanel(Spinewise::SpineParams& params);
} // namespace ui
