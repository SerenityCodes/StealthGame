#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace components {

struct Transform3D {
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    glm::mat4 as_matrix() const {
        glm::mat4<float> transform = glm::translate(glm::mat4(1.0f), translation);
        transform = glm::scale(transform, scale);
        return transform;
    }
};

}
