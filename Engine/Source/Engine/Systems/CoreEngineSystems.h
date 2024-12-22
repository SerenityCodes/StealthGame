#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Engine/Components/Components.h"
#include "Engine/Vulkan/Camera.h"
#include "Engine/Vulkan/VulkanRenderInfo.h"
#include "../Vendor/flecs/flecs.h"

namespace systems {

void setup_render_system(const flecs::world& world);

}