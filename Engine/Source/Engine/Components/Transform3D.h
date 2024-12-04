#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace components {

struct Transform3D {
    glm::vec3 translation;
    glm::vec3 rotation;
    glm::vec3 scale{1.0f, 1.0f, 1.0f};

    // Ry * Rx * Rz Tait Byran Rotation
    [[nodiscard]] glm::mat4 as_matrix() const {
        auto transform = glm::translate(glm::mat4(1.0f), translation);
        transform = glm::rotate(transform, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transform = glm::rotate(transform, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transform = glm::rotate(transform, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        transform = glm::scale(transform, scale);
        return transform;
    }
};

}
