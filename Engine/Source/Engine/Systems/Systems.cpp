#include "Systems.h"

#include <iostream>

#include "Engine/Engine.h"
#include "Engine/Components/Renderable.h"
#include "Engine/Components/Transform3D.h"
#include "Engine/Vulkan/VulkanRenderInfo.h"
#include "Engine/Vulkan/Camera.h"

namespace systems {

void render_system(const VulkanRenderInfo& render_info, components::Transform3D& transform, const components::Renderable& renderable, float delta_time, const Camera& camera) {
    const auto& [cmd_buffer, pipeline_layout] = render_info;
    constexpr float rotation_speed = 0.5f;
    transform.rotation.x = glm::mod(transform.rotation.x + delta_time * rotation_speed, glm::two_pi<float>());
    transform.rotation.y = glm::mod(transform.rotation.y + delta_time * rotation_speed * 2, glm::two_pi<float>());
    const PushConstantStruct push_constant{.transform = camera.get_projection() * transform.as_matrix()};
    vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantStruct), &push_constant);
    renderable.model->bind(cmd_buffer);
    renderable.model->draw(cmd_buffer);
}

}