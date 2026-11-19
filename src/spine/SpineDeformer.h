#pragma once

#include <vector>
#include <string>
#include <glm/vec3.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp> 
#include "SpineParams.h"

namespace Spinewise {

    struct Vertebra {
        int id;
        std::string name;
        Region region;

        glm::vec3 basePosition;
        glm::vec3 currentPosition;
        glm::vec3 rotationEuler;
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

        // TODO (Task 3): void applyCDeformation(const SpineParams& params);
        // TODO (Task 4): void applySDeformation(const SpineParams& params);

    private:
        std::vector<Vertebra> m_vertebrae;
    };

} // namespace Spinewise