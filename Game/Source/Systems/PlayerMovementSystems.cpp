﻿#include "PlayerMovementSystems.h"

#include <glm/ext/scalar_constants.hpp>

#include "Components/Components.h"
#include "Rendering/Camera.h"

void setup_keyboard_movement(flecs::world& world) {
    world.system<KeyboardMovement, components::Transform3D, Velocity>()
    .kind(flecs::OnUpdate)
    .each([](flecs::entity entity, KeyboardMovement& movement, components::Transform3D& transform, Velocity& velocity) {
        constexpr float move_speed = 3.f;
        constexpr float look_speed = 1.5f;
        glm::vec3 move_dir = glm::vec3{0.f};
        move_dir += static_cast<float>(movement.forward) * transform.forward();
        move_dir += static_cast<float>(movement.right) * transform.right();
        move_dir += static_cast<float>(movement.up) * transform.up();
        if (glm::dot(move_dir, move_dir) > glm::epsilon<float>()) {
            move_dir = glm::normalize(move_dir);
        }
        velocity.speed = move_speed;
        float delta_time = entity.world().delta_time();
        velocity.direction = move_dir * move_speed;
        transform.position += velocity.direction * delta_time;
        
        glm::vec3 rotate{0.f};
        rotate.x = static_cast<float>(movement.look_up);
        rotate.y = static_cast<float>(movement.look_right);
        if (glm::dot(rotate, rotate) > glm::epsilon<float>()) {
            transform.rotation += look_speed * delta_time * glm::normalize(rotate);
        }
        
        Camera* camera = entity.world().get_mut<Camera>();
        camera->set_view_yxz(transform.position, transform.rotation);
    });
}
