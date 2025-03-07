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
            engine::vulkan::VulkanRenderer::UniformBufferObject ubo{.model = transform.as_matrix(), .view = camera->get_view(), .projection = camera->get_projection()};
            
            renderable.model->bind(cmd_buffer);
            renderable.model->draw(cmd_buffer);  
        });
}

}