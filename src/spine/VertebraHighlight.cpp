#include "spine/VertebraHighlight.h"
#include <cmath>
#include <algorithm>

namespace Spinewise {

    bool SpineHighlightState::isVertebraAffected(int id) const {
        if (id < 0 || id >= static_cast<int>(vertebraHighlights.size())) return false;
        return vertebraHighlights[id].isAffected;
    }

    bool SpineHighlightState::isApex(int id) const {
        if (id < 0 || id >= static_cast<int>(vertebraHighlights.size())) return false;
        return vertebraHighlights[id].isApex;
    }

    VertebraHighlightType SpineHighlightState::getHighlightType(int id) const {
        if (id < 0 || id >= static_cast<int>(vertebraHighlights.size())) return VertebraHighlightType::None;
        return vertebraHighlights[id].type;
    }

    SpineHighlightState SpineHighlightEvaluator::evaluate(
        const SpineParams& params,
        const std::vector<Vertebra>& vertebrae,
        const HighlightParams& highlightParams)
    {
        SpineHighlightState state;
        const size_t count = vertebrae.size();
        state.vertebraHighlights.resize(count);

        for (size_t i = 0; i < count; ++i) {
            state.vertebraHighlights[i].vertebraId = vertebrae[i].id;
            state.vertebraHighlights[i].name = vertebrae[i].name;
            state.vertebraHighlights[i].type = VertebraHighlightType::None;
            state.vertebraHighlights[i].isAffected = false;
            state.vertebraHighlights[i].isApex = false;

            // Default base color matching SpineLabScreen (Lumbar cyan, Thoracic bone white, Cervical amber)
            if (vertebrae[i].id <= 4)
                state.vertebraHighlights[i].color = glm::vec3(0.2f, 0.8f, 0.9f);
            else if (vertebrae[i].id <= 16)
                state.vertebraHighlights[i].color = glm::vec3(0.9f, 0.9f, 0.95f);
            else
                state.vertebraHighlights[i].color = glm::vec3(1.0f, 0.7f, 0.4f);
        }

        // 1. Placeholder / manual testing mode
        if (highlightParams.usePlaceholderData) {
            state.primaryConvexDirection = highlightParams.placeholderConvexDirection;
            state.primaryConcaveDirection = (highlightParams.placeholderConvexDirection == Direction::Right)
                ? Direction::Left : Direction::Right;

            for (int id : highlightParams.placeholderAffectedIds) {
                if (id >= 0 && id < static_cast<int>(count)) {
                    state.vertebraHighlights[id].isAffected = true;
                    state.vertebraHighlights[id].type = VertebraHighlightType::Affected;
                    if (highlightParams.enableVertebraHighlight) {
                        state.vertebraHighlights[id].color = highlightParams.affectedColor;
                    }
                }
            }

            int apexId = highlightParams.placeholderApexId;
            if (apexId >= 0 && apexId < static_cast<int>(count)) {
                state.primaryApexId = apexId;
                state.primaryApexName = vertebrae[apexId].name;
                state.vertebraHighlights[apexId].isApex = true;
                state.vertebraHighlights[apexId].type = VertebraHighlightType::Apex;
                if (highlightParams.enableApexHighlight) {
                    state.vertebraHighlights[apexId].color = highlightParams.apexColor;
                }
            }
            state.affectedRangeText = "Placeholder (Mode Testing)";
            return state;
        }

        // 2. Real deformation-driven highlight evaluation
        state.primaryConvexDirection = params.primaryDirection;
        state.primaryConcaveDirection = (params.primaryDirection == Direction::Right)
            ? Direction::Left : Direction::Right;

        if (!params.isScoliosis() && params.primaryCobb <= 0.001f) {
            state.affectedRangeText = "Tidak Ada Kurva Skoliosis";
            return state;
        }

        // Determine affected range for Primary Curve
        int pStart = 5, pEnd = 16;
        if (params.primaryRegion == Region::Lumbar) { pStart = 0; pEnd = 4; }
        else if (params.primaryRegion == Region::Thoracic) { pStart = 5; pEnd = 16; }
        else if (params.primaryRegion == Region::Thoracolumbar) { pStart = 2; pEnd = 11; }

        for (int i = pStart; i <= pEnd; ++i) {
            if (i >= 0 && i < static_cast<int>(count)) {
                state.vertebraHighlights[i].isAffected = true;
                state.vertebraHighlights[i].type = VertebraHighlightType::Affected;
            }
        }

        // Threshold minimum untuk membedakan deformasi aktif dari zero-displacement
        constexpr float kMinDisplacementThreshold = 1e-4f;

        // Determine Primary Apex (highest curve displacement in target position)
        float maxDisplacement = kMinDisplacementThreshold;
        int primaryApex = -1;
        for (int i = pStart; i <= pEnd; ++i) {
            if (i >= 0 && i < static_cast<int>(count)) {
                float disp = std::abs(vertebrae[i].targetPosition.x - vertebrae[i].basePosition.x);
                if (disp > maxDisplacement) {
                    maxDisplacement = disp;
                    primaryApex = i;
                }
            }
        }
        if (primaryApex == -1 && pStart <= pEnd) {
            primaryApex = (pStart + pEnd) / 2; // Midpoint fallback saat zero displacement
        }

        if (primaryApex >= 0 && primaryApex < static_cast<int>(count)) {
            state.primaryApexId = primaryApex;
            state.primaryApexName = vertebrae[primaryApex].name;
            state.vertebraHighlights[primaryApex].isApex = true;
            state.vertebraHighlights[primaryApex].type = VertebraHighlightType::Apex;
        }

        // Secondary curve handling for S-shaped pattern (menjamin tidak ada overlap dengan primary region)
        if (params.pattern == CurvePattern::S && params.secondaryCobb > 0.001f) {
            int sStart = 0, sEnd = 4;
            if (params.primaryRegion == Region::Lumbar) {
                sStart = 5; sEnd = 16;
            } else if (params.primaryRegion == Region::Thoracolumbar) {
                sStart = 12; sEnd = 16; // Upper Thoracic (disjoint dari Thoracolumbar 2..11)
            } else {
                sStart = 0; sEnd = 4;   // Lumbar (disjoint dari Thoracic 5..16)
            }

            for (int i = sStart; i <= sEnd; ++i) {
                if (i >= 0 && i < static_cast<int>(count)) {
                    state.vertebraHighlights[i].isAffected = true;
                    if (state.vertebraHighlights[i].type == VertebraHighlightType::None) {
                        state.vertebraHighlights[i].type = VertebraHighlightType::Affected;
                    }
                }
            }

            float maxSDisp = kMinDisplacementThreshold;
            int secondaryApex = -1;
            for (int i = sStart; i <= sEnd; ++i) {
                if (i >= 0 && i < static_cast<int>(count)) {
                    float disp = std::abs(vertebrae[i].targetPosition.x - vertebrae[i].basePosition.x);
                    if (disp > maxSDisp) {
                        maxSDisp = disp;
                        secondaryApex = i;
                    }
                }
            }
            if (secondaryApex == -1 && sStart <= sEnd) {
                secondaryApex = (sStart + sEnd) / 2;
            }

            if (secondaryApex >= 0 && secondaryApex < static_cast<int>(count)) {
                state.secondaryApexId = secondaryApex;
                state.secondaryApexName = vertebrae[secondaryApex].name;
                state.vertebraHighlights[secondaryApex].isApex = true;
                state.vertebraHighlights[secondaryApex].type = VertebraHighlightType::Apex;
            }
        }

        // Assign colors based on final priority (Apex > Affected > Base)
        for (size_t i = 0; i < count; ++i) {
            auto& vInfo = state.vertebraHighlights[i];
            if (vInfo.isApex && highlightParams.enableApexHighlight) {
                vInfo.color = highlightParams.apexColor;
            } else if (vInfo.isAffected && highlightParams.enableVertebraHighlight) {
                vInfo.color = highlightParams.affectedColor;
            }
        }

        // Format affected range text
        if (pStart < static_cast<int>(count) && pEnd < static_cast<int>(count)) {
            state.affectedRangeText = vertebrae[pEnd].name + " - " + vertebrae[pStart].name;
        }

        return state;
    }

} // namespace Spinewise
