#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Components/Components.h"
#include "Vulkan/Camera.h"
#include "Vulkan/VulkanRenderInfo.h"
#include "../Vendor/flecs/flecs.h"

namespace systems {

void setup_render_system(const flecs::world& world);

}