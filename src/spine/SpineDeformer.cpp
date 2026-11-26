#include "SpineDeformer.h"
#include <cmath>
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

            if (i <= 4) v.region = VertebraRegion::Lumbar;          // L5-L1
            else if (i <= 16) v.region = VertebraRegion::Thoracic;  // T12-T1
            else v.region = VertebraRegion::Cervical;               // C7-C1

            float y = startY + (i * vertebraSpacing);
            float x = 0.0f;
            float t = static_cast<float>(i) / 23.0f;

            // Kurva alami bidang sagital (Lordosis & Kifosis)
            float z = 0.0f;
            if (t <= 0.25f) {
                float localT = t / 0.25f;
                z = -0.5f * std::sin(localT * 3.14159f);       // Lumbar Lordosis
            }
            else if (t <= 0.80f) {
                float localT = (t - 0.25f) / 0.55f;
                z = 0.8f * std::sin(localT * 3.14159f);        // Thoracic Kyphosis
            }
            else {
                float localT = (t - 0.80f) / 0.20f;
                z = -0.3f * std::sin(localT * 3.14159f);       // Cervical Lordosis
            }

            v.basePosition = glm::vec3(x, y, z);
            v.currentPosition = v.basePosition;
            v.rotationEuler = glm::vec3(0.0f);

            float sizeY = vertebraSpacing * 0.48f;
            float sizeXZ = 0.8f - (t * 0.3f);
            v.scale = glm::vec3(sizeXZ, sizeY, sizeXZ);
        }
    }
    void SpineDeformer::applyCDeformation(const SpineParams& params)
    {
        for (auto& v : m_vertebrae) {
            v.currentPosition = v.basePosition;
            v.rotationEuler = glm::vec3(0.0f);
        }
        if (params.primaryCobb <= 0.001f) {
            return;
        }
        int startIdx = 5;  // Default Thoracic
        int endIdx = 16;

        if (params.primaryRegion == Region::Lumbar) {
            startIdx = 0;
            endIdx = 4;
        }
        else if (params.primaryRegion == Region::Thoracic) {
            startIdx = 5;
            endIdx = 16;
        }
        else if (params.primaryRegion == Region::Thoracolumbar) {
            startIdx = 2;
            endIdx = 11;
        }

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

            float t = (v.basePosition.y - yLower) / L;
            t = std::clamp(t, 0.0f, 1.0f);

            float offsetX = s * A * std::sin(PI * t);
            v.currentPosition.x = v.basePosition.x + offsetX;

            float dx_dy = s * A * (PI / L) * std::cos(PI * t);
            v.rotationEuler.z = -std::atan(dx_dy);
        }
    }

} // namespace Spinewise