#include "UpdateCameraSystem.h"

#include "Engine/Components/Renderable.h"
#include "Engine/Components/Transform3D.h"
#include "Engine/Vulkan/Camera.h"

void UpdateCameraSystem::update_camera_system(entt::registry& registry, GLFWwindow* window, float dt) {
    auto viewer_entity_view = registry.view<Camera, components::Transform3D>(entt::exclude<components::Renderable>);
    for (auto entity : viewer_entity_view) {
        Camera& camera = registry.get<Camera>(entity);
        auto& transform = registry.get<components::Transform3D>(entity);
        camera.set_view_yxz(transform.translation, transform.rotation);
    }
}