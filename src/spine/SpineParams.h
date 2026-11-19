#pragma once

#include <algorithm>

namespace Spinewise {
    // (Diny) Fitur Spine Data Contract
    enum class CurvePattern {
        C,
        S
    };

    enum class Region {
        Thoracic,
        Thoracolumbar,
        Lumbar
    };

    enum class Direction {
        Left,
        Right
    };

    struct SpineParams {
        CurvePattern pattern = CurvePattern::C;
        Region       primaryRegion = Region::Thoracic;
        Direction    primaryDirection = Direction::Right;

        float        primaryCobb = 0.0f;

        float        secondaryCobb = 0.0f;

        void clampValues() {
            primaryCobb = std::clamp(primaryCobb, 0.0f, 70.0f);
            secondaryCobb = std::clamp(secondaryCobb, 0.0f, 70.0f);
        }

        bool isScoliosis() const {
            return primaryCobb >= 10.0f || (pattern == CurvePattern::S && secondaryCobb >= 10.0f);
        }
    };

} // namespace Spinewise