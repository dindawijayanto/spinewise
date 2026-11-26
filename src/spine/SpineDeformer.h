#pragma once

#include <vector>
#include <string>

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "spine/SpineParams.h"

namespace Spinewise {

    enum class VertebraRegion {
        Lumbar,
        Thoracic,
        Cervical
    };

    struct Vertebra {
        int id;
        std::string name;
        VertebraRegion region;

        glm::vec3 basePosition;      // Posisi lurus normal
        glm::vec3 startPosition;     // Titik awal animasi
        glm::vec3 targetPosition;    // Titik target deformasi
        glm::vec3 currentPosition;   // Posisi aktif di layar

        glm::vec3 startRotation;
        glm::vec3 targetRotation;
        glm::vec3 rotationEuler;     // Rotasi aktif di layar

        glm::vec3 scale;

        glm::mat4 getModelMatrix() const {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, currentPosition);
            model = glm::rotate(model, rotationEuler.z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, rotationEuler.x, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, rotationEuler.y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, scale);
            return model;
        }
    };

    class SpineDeformer {
    public:
        SpineDeformer();

        void initializeReferenceCurve();
        const std::vector<Vertebra>& getVertebrae() const { return m_vertebrae; }

        void applyDeformation(const SpineParams& params, bool animate = false);
        void update(float dt);

    private:
        void calculateCDeformation(const SpineParams& params);
        void calculateSDeformation(const SpineParams& params);

        std::vector<Vertebra> m_vertebrae;

        bool m_isAnimating = false;
        float m_animTime = 0.0f;
        const float m_animDuration = 1.5f;
    };

} // namespace Spinewise