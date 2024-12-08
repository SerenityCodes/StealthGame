#include "Systems.h"

#include "Engine/Engine.h"
#include "Engine/Components/Renderable.h"
#include "Engine/Components/Transform3D.h"
#include "Engine/Vulkan/VulkanRenderInfo.h"

namespace systems {

void setup_render_system(flecs::world& world) {
    world.system<components::Renderable, components::Transform3D>()
        .kind(flecs::PostUpdate)
        .each([](flecs::entity e, const components::Renderable& renderable, components::Transform3D& transform) {
            const auto& [cmd_buffer, pipeline_layout] = *e.world().get<VulkanRenderInfo>();
            constexpr float rotation_speed = 0.005f;
            transform.rotation = {
                glm::mod(transform.rotation.x * e.world().delta_time() * rotation_speed / 2, glm::two_pi<float>()),
                glm::mod(transform.rotation.y * e.world().delta_time() * rotation_speed, glm::two_pi<float>()),
                transform.rotation.z
            };
            const PushConstantStruct push_constant{transform.as_matrix()};
            vkCmdPushConstants(cmd_buffer, pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantStruct), &push_constant);
            renderable.model->bind(cmd_buffer);
            renderable.model->draw(cmd_buffer);
        });
}

}