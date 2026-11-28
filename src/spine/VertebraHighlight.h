#pragma once

#include "spine/SpineParams.h"
#include "spine/SpineDeformer.h"
#include <glm/vec3.hpp>
#include <vector>
#include <string>

namespace Spinewise {

    enum class VertebraHighlightType {
        None = 0,
        Affected,
        Apex
    };

    enum class CurveSide {
        None = 0,
        Convex,
        Concave
    };

    struct HighlightParams {
        bool enableVertebraHighlight = true;
        bool enableApexHighlight = true;
        bool enableSideVisualization = true;

        glm::vec3 affectedColor{1.0f, 0.65f, 0.0f};  // Amber / Orange
        glm::vec3 apexColor{0.95f, 0.20f, 0.20f};     // Red / Crimson
        glm::vec3 convexColor{0.20f, 0.70f, 1.0f};   // Cyan / Blue
        glm::vec3 concaveColor{0.80f, 0.30f, 0.90f};  // Purple / Magenta

        // Controls for testing / placeholder verification
        bool usePlaceholderData = false;
        std::vector<int> placeholderAffectedIds{7, 8, 9, 10, 11};
        int placeholderApexId = 9;
        Direction placeholderConvexDirection = Direction::Right;
    };

    struct VertebraHighlightInfo {
        int vertebraId = -1;
        std::string name;
        VertebraHighlightType type = VertebraHighlightType::None;
        glm::vec3 color{1.0f};
        bool isAffected = false;
        bool isApex = false;
    };

    struct SpineHighlightState {
        std::vector<VertebraHighlightInfo> vertebraHighlights; // 24 vertebrae
        Direction primaryConvexDirection = Direction::Right;
        Direction primaryConcaveDirection = Direction::Left;
        int primaryApexId = -1;
        int secondaryApexId = -1;
        std::string primaryApexName;
        std::string secondaryApexName;
        std::string affectedRangeText;

        bool isVertebraAffected(int id) const;
        bool isApex(int id) const;
        VertebraHighlightType getHighlightType(int id) const;
    };

    class SpineHighlightEvaluator {
    public:
        static SpineHighlightState evaluate(
            const SpineParams& params,
            const std::vector<Vertebra>& vertebrae,
            const HighlightParams& highlightParams
        );
    };

} // namespace Spinewise
