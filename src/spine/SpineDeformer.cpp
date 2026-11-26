#include "SpineDeformer.h"
#include <cmath>
#include <algorithm>

namespace Spinewise {

    SpineDeformer::SpineDeformer() {
        m_vertebrae.resize(24);
        initializeReferenceCurve();
    }

    void SpineDeformer::initializeReferenceCurve() {
        const std::vector<std::string> names = {
            "L5", "L4", "L3", "L2", "L1",
            "T12", "T11", "T10", "T9", "T8", "T7",
            "T6", "T5", "T4", "T3", "T2", "T1",
            "C7", "C6", "C5", "C4", "C3", "C2", "C1"
        };

        const float totalHeight = 12.0f;
        const float vertebraSpacing = totalHeight / 24.0f;
        const float startY = -totalHeight / 2.0f;

        for (int i = 0; i < 24; ++i) {
            Vertebra& v = m_vertebrae[i];
            v.id = i;
            v.name = names[i];

            if (i <= 4) v.region = VertebraRegion::Lumbar;
            else if (i <= 16) v.region = VertebraRegion::Thoracic;
            else v.region = VertebraRegion::Cervical;

            float y = startY + (i * vertebraSpacing);
            float x = 0.0f;
            float t = static_cast<float>(i) / 23.0f;

            float z = 0.0f;
            if (t <= 0.25f) {
                float localT = t / 0.25f;
                z = -0.5f * std::sin(localT * 3.14159f);
            }
            else if (t <= 0.80f) {
                float localT = (t - 0.25f) / 0.55f;
                z = 0.8f * std::sin(localT * 3.14159f);
            }
            else {
                float localT = (t - 0.80f) / 0.20f;
                z = -0.3f * std::sin(localT * 3.14159f);
            }

            v.basePosition = glm::vec3(x, y, z);
            v.currentPosition = v.basePosition;
            v.rotationEuler = glm::vec3(0.0f);

            v.targetPosition = v.basePosition;
            v.startPosition = v.basePosition;
            v.startRotation = glm::vec3(0.0f);
            v.targetRotation = glm::vec3(0.0f);

            float sizeY = vertebraSpacing * 0.48f;
            float sizeXZ = 0.8f - (t * 0.3f);
            v.scale = glm::vec3(sizeXZ, sizeY, sizeXZ);
        }
    }

    void SpineDeformer::applyDeformation(const SpineParams& params, bool animate) {
        if (params.pattern == CurvePattern::C) {
            calculateCDeformation(params);
        }
        else {
            calculateSDeformation(params);
        }

        if (animate) {
            for (auto& v : m_vertebrae) {
                v.startPosition = v.basePosition;
                v.startRotation = glm::vec3(0.0f);
                v.currentPosition = v.basePosition;
                v.rotationEuler = glm::vec3(0.0f);
            }
            m_isAnimating = true;
            m_animTime = 0.0f;
        }
        else {
            m_isAnimating = false;
            for (auto& v : m_vertebrae) {
                v.currentPosition = v.targetPosition;
                v.rotationEuler = v.targetRotation;
            }
        }
    }

    void SpineDeformer::calculateCDeformation(const SpineParams& params) {
        if (params.primaryCobb <= 0.001f) return;

        int startIdx = 5, endIdx = 16;
        if (params.primaryRegion == Region::Lumbar) { startIdx = 0; endIdx = 4; }
        else if (params.primaryRegion == Region::Thoracic) { startIdx = 5; endIdx = 16; }
        else if (params.primaryRegion == Region::Thoracolumbar) { startIdx = 2; endIdx = 11; }

        float yLower = m_vertebrae[startIdx].basePosition.y;
        float yUpper = m_vertebrae[endIdx].basePosition.y;
        float L = std::abs(yUpper - yLower);
        if (L <= 0.001f) return;

        const float PI = 3.14159265359f;
        float thetaRad = params.primaryCobb * (PI / 180.0f);
        float A = (L / PI) * std::tan(thetaRad / 2.0f);
        float s = (params.primaryDirection == Direction::Right) ? -1.0f : 1.0f;

        for (int i = startIdx; i <= endIdx; ++i) {
            Vertebra& v = m_vertebrae[i];
            float t = std::clamp((v.basePosition.y - yLower) / L, 0.0f, 1.0f);

            v.targetPosition.x = v.basePosition.x + (s * A * std::sin(PI * t));
            float dx_dy = s * A * (PI / L) * std::cos(PI * t);
            v.targetRotation.z = -std::atan(dx_dy);
        }
    }

    void SpineDeformer::calculateSDeformation(const SpineParams& params) {
        const float PI = 3.14159265359f;

        int pStart = 5, pEnd = 16;
        int sStart = 0, sEnd = 4;

        if (params.primaryRegion == Region::Lumbar) {
            pStart = 0; pEnd = 4;
            sStart = 5; sEnd = 16;
        }

        float s1 = (params.primaryDirection == Direction::Right) ? -1.0f : 1.0f;
        float s2 = -s1;

        if (params.primaryCobb > 0.001f) {
            float yLower = m_vertebrae[pStart].basePosition.y;
            float yUpper = m_vertebrae[pEnd].basePosition.y;
            float L1 = std::abs(yUpper - yLower);
            if (L1 > 0.001f) {
                float theta1 = params.primaryCobb * (PI / 180.0f);
                float A1 = (L1 / PI) * std::tan(theta1 / 2.0f);

                for (int i = pStart; i <= pEnd; ++i) {
                    Vertebra& v = m_vertebrae[i];
                    float t = std::clamp((v.basePosition.y - yLower) / L1, 0.0f, 1.0f);
                    v.targetPosition.x = v.basePosition.x + (s1 * A1 * std::sin(PI * t));
                    float dx_dy = s1 * A1 * (PI / L1) * std::cos(PI * t);
                    v.targetRotation.z = -std::atan(dx_dy);
                }
            }
        }

        if (params.pattern == CurvePattern::S && params.secondaryCobb > 0.001f) {
            int sStart_idx = 0, sEnd_idx = 4;
            if (params.primaryRegion == Region::Lumbar) { sStart_idx = 5; sEnd_idx = 16; }

            float yLower = m_vertebrae[sStart_idx].basePosition.y;
            float yUpper = m_vertebrae[sEnd_idx].basePosition.y;
            float L2 = std::abs(yUpper - yLower);
            if (L2 > 0.001f) {
                float theta2 = params.secondaryCobb * (PI / 180.0f);
                float A2 = (L2 / PI) * std::tan(theta2 / 2.0f);

                for (int i = sStart_idx; i <= sEnd_idx; ++i) {
                    Vertebra& v = m_vertebrae[i];
                    float t = std::clamp((v.basePosition.y - yLower) / L2, 0.0f, 1.0f);
                    v.targetPosition.x = v.basePosition.x + (s2 * A2 * std::sin(PI * t));
                    float dx_dy = s2 * A2 * (PI / L2) * std::cos(PI * t);
                    v.targetRotation.z = -std::atan(dx_dy);
                }
            }
        }
    }

    void SpineDeformer::update(float dt) {
        if (!m_isAnimating) return;

        m_animTime += dt;
        float progress = std::clamp(m_animTime / m_animDuration, 0.0f, 1.0f);

        const float PI = 3.14159265359f;
        float ease = 0.5f * (1.0f - std::cos(progress * PI));

        for (auto& v : m_vertebrae) {
            v.currentPosition = glm::mix(v.startPosition, v.targetPosition, ease);
            v.rotationEuler = glm::mix(v.startRotation, v.targetRotation, ease);
        }

        if (progress >= 1.0f) {
            m_isAnimating = false;
        }
    }

} // namespace Spinewise