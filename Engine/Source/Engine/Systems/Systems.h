#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "Engine/ECS/Components/Components.h"
#include "Engine/Vulkan/Camera.h"
#include "Engine/Vulkan/VulkanRenderInfo.h"

namespace systems {

struct PushConstantStruct {
    glm::mat4 transform;
};

void render_system(const VulkanRenderInfo& render_info, components::Transform3D& transform, const components::Renderable& renderable, float delta_time, const Camera& camera);

}