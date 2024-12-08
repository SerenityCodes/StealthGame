#pragma once

#include <glm/glm.hpp>

#include "../Vendor/flecs/flecs.h"
#include "Engine/Vulkan/Wrappers/CommandBufferWrapper.h"

namespace systems {

struct PushConstantStruct {
    glm::mat4 transform;
};

void setup_render_system(flecs::world& world);

}