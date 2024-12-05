#pragma once
#include "../../../Vendor/entt/include/entt.hpp"
#include "GLFW/glfw3.h"

namespace engine {

class KeyboardMovementSystem {
public:
    struct KeyMappings {
        int move_left = GLFW_KEY_A;
        int move_right = GLFW_KEY_D;
        int move_forward = GLFW_KEY_W;
        int move_back = GLFW_KEY_S;
        int move_up = GLFW_KEY_E;
        int move_down = GLFW_KEY_Q;
        int look_left = GLFW_KEY_LEFT;
        int look_right = GLFW_KEY_RIGHT;
        int look_up = GLFW_KEY_UP;
        int look_down = GLFW_KEY_DOWN;
    };

    void move_in_plane_xz(GLFWwindow* window, float dt, entt::registry& registry) const;

    KeyMappings keyboard{};
    float move_speed{3.f};
    float look_speed{1.5f};
};

}
