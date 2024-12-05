#pragma once
#include "../../../Vendor/entt/include/entt.hpp"
#include "Engine/Vulkan/Wrappers/Window.h"

struct UpdateCameraSystem {
    static void update_camera_system(entt::registry& registry, GLFWwindow* window, float dt);
};
