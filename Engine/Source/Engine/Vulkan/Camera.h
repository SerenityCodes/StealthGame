#pragma once
#include <glm/glm.hpp>

class Camera {
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
public:
    Camera() = default;
    Camera(float fov_y, float aspect, float near, float far);
    void set_orthographic_projection(float left, float right, float top, float bottom, float near, float far);
    void set_perspective_projection(float fov_y, float aspect, float near, float far);
    void set_view_direction(glm::vec3 position, glm::vec3 direction, glm::vec3 up = {0.f, -1.f, 0.f});
    void set_view_target(glm::vec3 position, glm::vec3 target, glm::vec3 up = {0.f, -1.f, 0.f});
    void set_view_yxz(glm::vec3 position, glm::vec3 rotation);

    const glm::mat4& get_projection() const;
    const glm::mat4& get_view() const;
};
