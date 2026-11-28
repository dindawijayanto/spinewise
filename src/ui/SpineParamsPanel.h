#pragma once

#include "spine/SpineParams.h"
#include "spine/VertebraHighlight.h"

namespace ui
{
// Menggambar panel input parameter skoliosis + kontrol highlight.
// Return true kalau ada nilai yang berubah di frame ini.
bool drawSpineParamsPanel(Spinewise::SpineParams& params,
                          Spinewise::HighlightParams& highlightParams,
                          const Spinewise::SpineHighlightState& highlightState);
} // namespace ui
