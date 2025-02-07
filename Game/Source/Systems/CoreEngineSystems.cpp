#include "CoreEngineSystems.h"

#include "Vulkan/Wrappers/PipelineWrapper.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

namespace systems {

void setup_render_system(const flecs::world& world) {
    world.system<components::Transform3D, components::Renderable>()
        .kind(flecs::PostUpdate)
        .each([](flecs::entity entity, components::Transform3D& transform, components::Renderable& renderable) {
            const flecs::world ecs_world = entity.world();
            const auto& [cmd_buffer, pipeline_layout] = *ecs_world.get<VulkanRenderInfo>();
            const Camera* camera = ecs_world.get<Camera>();
            glm::mat4 projection_view = camera->get_projection() * camera->get_view();
            const engine::vulkan::PipelineWrapper::SimplePushConstantData push_constant{.transform = projection_view * transform.as_matrix(), .normal = transform.normal_matrix()};
            vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(engine::vulkan::PipelineWrapper::SimplePushConstantData), &push_constant);
            renderable.model->bind(cmd_buffer);
            renderable.model->draw(cmd_buffer);  
        });
}

}