#include "CoreEngineSystems.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

namespace systems {

/*
void setup_render_system(const flecs::world& world) {
    world.system<components::Transform3D, components::Renderable>()
        .kind(flecs::PostUpdate)
        .each([](flecs::entity entity, components::Transform3D& transform, components::Renderable& renderable) {
            const flecs::world ecs_world = entity.world();
            const auto& [frame, pipeline_layout, renderer] = *ecs_world.get<VulkanRenderInfo>();
            frame->ubo.model = transform.as_matrix();
            frame->ubo.normal_matrix = transform.normal_matrix();
            frame->staging_uniform_buffer.write_to_buffer(&frame->ubo, sizeof(engine::vulkan::VulkanRenderer::UniformBufferObject));
            renderer->copy_buffer(frame->staging_uniform_buffer.buffer(), frame->uniform_buffer.buffer(), sizeof(engine::vulkan::VulkanRenderer::UniformBufferObject));
            renderable.model->bind(frame->command_buffer);
            renderable.model->draw(frame->command_buffer);  
        });
}
*/

}