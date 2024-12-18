#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Engine/ECS/Components/Components.h"
#include "Engine/Vulkan/Camera.h"
#include "Engine/Vulkan/VulkanRenderInfo.h"
#include "../Vendor/flecs/flecs.h"

namespace systems {

struct PushConstantStruct {
    glm::mat4 transform;
};

void setup_render_system(const flecs::world& world);

}