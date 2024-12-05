#include "KeyboardMovementSystem.h"

#include <glm/vec3.hpp>

#include "../../../Vendor/entt/include/entt.hpp"
#include "Engine/Components/Transform3D.h"
#include "Engine/Vulkan/Camera.h"

namespace engine {

void KeyboardMovementSystem::move_in_plane_xz(GLFWwindow* window,
    float dt, entt::registry& registry) const {
    glm::vec3 rotate{0};
    if (glfwGetKey(window, keyboard.look_right) == GLFW_PRESS) {
        rotate.y += 1.f;
    }
    if (glfwGetKey(window, keyboard.look_left) == GLFW_PRESS) {
        rotate.y -= 1.f;
    }
    if (glfwGetKey(window, keyboard.look_up) == GLFW_PRESS) {
        rotate.x += 1.f;
    }
    if (glfwGetKey(window, keyboard.look_down) == GLFW_PRESS) {
        rotate.x -= 1.f;
    }
    auto transforms_view = registry.view<components::Transform3D>(entt::exclude<Camera>);
    for (auto entity : transforms_view) {
        components::Transform3D& transform = registry.get<components::Transform3D>(entity);
        if (dot(rotate, rotate) > glm::epsilon<float>()) {
            transform.rotation += look_speed * dt * normalize(rotate);
        }
        transform.rotation.x = glm::clamp(transform.rotation.x, -1.5f, 1.5f);
        transform.rotation.y = glm::mod(transform.rotation.y, glm::two_pi<float>());

        float yaw = transform.rotation.y;
        const glm::vec3 forward = {sin(yaw), 0.0f, cos(yaw)};
        const glm::vec3 right = {forward.z, 0.0f, -forward.z};
        glm::vec3 up = {0.f, 01.f, 0.f};

        glm::vec3 move_dir{0.f};
        if (glfwGetKey(window, keyboard.move_forward) == GLFW_PRESS) {
            move_dir += forward;
        }
        if (glfwGetKey(window, keyboard.move_back) == GLFW_PRESS) {
            move_dir -= forward;
        }
        if (glfwGetKey(window, keyboard.move_right) == GLFW_PRESS) {
            move_dir += right;
        }
        if (glfwGetKey(window, keyboard.move_left) == GLFW_PRESS) {
            move_dir -= right;
        }
        if (glfwGetKey(window, keyboard.move_up) == GLFW_PRESS) {
            move_dir += up;
        }
        if (glfwGetKey(window, keyboard.move_down) == GLFW_PRESS) {
            move_dir -= up;
        }
        transform.translation += move_speed * dt * normalize(move_dir);
    }
}

}