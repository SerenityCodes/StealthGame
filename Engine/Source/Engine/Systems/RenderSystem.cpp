#include "RenderSystem.h"

#include "Engine/Components/Renderable.h"
#include "Engine/Components/Transform3D.h"
#include "Engine/Vulkan/Camera.h"
#include "Engine/Vulkan/Wrappers/PipelineWrapper.h"

namespace engine {

void RenderSystem::render_entities(entt::registry& registry, VkCommandBuffer cmd_buffer, VkPipelineLayout pipeline_layout, float aspect) {
    using namespace components;
    auto renderer_view = registry.view<Renderable, Transform3D>();
    for (auto renderable_entity : renderer_view) {
        const vulkan::VulkanModel* model_ptr = renderer_view.get<Renderable>(renderable_entity).model;
        vulkan::PipelineWrapper::SimplePushConstantData push_data{};
        auto& transform = registry.get<Transform3D>(renderable_entity);
        push_data.transform = transform.as_matrix();
        vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vulkan::PipelineWrapper::SimplePushConstantData), &push_data);
        model_ptr->bind(cmd_buffer);
        model_ptr->draw(cmd_buffer);
    }
}

}