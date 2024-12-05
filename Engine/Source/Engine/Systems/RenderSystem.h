#pragma once

#include <../Vendor/entt/include/entt.hpp>

#include "Engine/Vulkan/Wrappers/CommandBufferWrapper.h"

namespace engine {

struct RenderSystem {
    static void render_entities(entt::registry& registry, VkCommandBuffer cmd_buffer, VkPipelineLayout pipeline_layout, float aspect);
};

}
