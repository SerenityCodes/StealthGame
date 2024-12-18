#include "CoreEngineSystems.h"

namespace systems {

void setup_render_system(const flecs::world& world) {
    world.system<components::Transform3D, components::Renderable>()
        .kind(flecs::PostUpdate)
        .each([](flecs::entity entity, components::Transform3D& transform, components::Renderable& renderable) {
            const flecs::world ecs_world = entity.world();
            const auto& [cmd_buffer, pipeline_layout] = *ecs_world.get<VulkanRenderInfo>();
            const Camera* camera = ecs_world.get<Camera>();
            const PushConstantStruct push_constant{.transform = camera->get_projection() * transform.as_matrix()};
            vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantStruct), &push_constant);
            renderable.model->bind(cmd_buffer);
            renderable.model->draw(cmd_buffer);  
        });
}

}