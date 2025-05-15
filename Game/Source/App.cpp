#include <random>

#include "Systems/PlayerMovementSystems.h"
#include "Engine.h"
#include "Components/Components.h"
#include "Systems/CoreEngineSystems.h"
#include "Vulkan/Camera.h"
#include "Vulkan/VulkanModel.h"

glm::vec3 get_random_direction() {
    static std::mt19937_64 gen{std::random_device{}()};
    static std::uniform_real_distribution dist(-1.f, 1.f);

    glm::vec3 direction = glm::vec3(dist(gen), dist(gen), dist(gen));
    if (glm::length(direction) > 0.0f) {
        direction = glm::normalize(direction);
    }
    return direction;
}

void rotate_system(flecs::entity entity, components::Transform3D& transform, Velocity& velocity, components::Renderable& renderable) {
    constexpr float rotation_speed = 0.5f;
    constexpr float fall_speed = 0.01f;
    float delta_time = entity.world().delta_time();
    transform.rotation.x = glm::mod(transform.rotation.x + delta_time * rotation_speed, glm::two_pi<float>());
    transform.rotation.y = glm::mod(transform.rotation.y + delta_time * rotation_speed * 2, glm::two_pi<float>());

    // Send it in a random direction
    transform.position += velocity.speed * velocity.direction * delta_time;
    if (transform.position.x > 1.0f || transform.position.x < -1.0f || transform.position.y > 1.0f || transform.position.y < -1.0f) {
        entity.destruct();
    } 
}

void spawn_and_move_cube(const flecs::world& world) {
    world.system<components::Transform3D, Velocity, components::Renderable>()
        .kind(flecs::OnUpdate)
        .each(rotate_system);
    world.system<components::Renderable>()
        .with(flecs::Prefab)
        .kind(flecs::OnUpdate)
        .each([](flecs::entity entity, components::Renderable& renderable) {
            static float timer = 0.0f;
            timer += entity.world().delta_time();
            if (timer >= 1.0f) {
               timer = 0.0f;
               auto world = entity.world();
               flecs::entity cube = world.entity().is_a(entity);
               cube.set<components::Transform3D>({.position  = {0.f, 0.f, 2.5f}, .rotation = {0.f, 0.f, 0.f}, .scale = {.5f, .5f, .5f}})
                   .set<components::Renderable>({renderable.model})
                   .set<Velocity>({.direction = get_random_direction(), .speed = .5f});
           }
        });
}

void initialize_world(const flecs::world& world, ArrayRef<engine::vulkan::VulkanModel*> models, float aspect) {
    world.emplace<Camera>(glm::radians(45.0f), aspect, 0.1f, 10.f);
    for (engine::vulkan::VulkanModel* model : models) {
        flecs::entity cube = world.prefab();
        cube.set<components::Transform3D>({.position  = {0.f, 0.f, 2.5f}, .rotation = {0.f, 0.f, 0.f}, .scale = glm::vec3{.5f}});
        cube.set<components::Renderable>({model});
        cube.set<Velocity>({.direction = get_random_direction(), .speed = .5f});
    }
    spawn_and_move_cube(world);
    const flecs::entity viewer_entity = world.entity();
    viewer_entity
        .set<components::Transform3D>({glm::vec3{0.f}, glm::vec3{0.f}, glm::vec3{1.f}})
        .add<KeyboardMovement>()
        .set<Velocity>({.direction = glm::vec3{0.f}, .speed = 0.f});
}

int main() {
	engine::StealthEngine engine;
    flecs::world& world = engine.get_world();
    // For more info on import flags, go here
    // https://github.com/assimp/assimp/blob/master/include/assimp/postprocess.h
    engine::vulkan::VulkanModel vase_model = engine.load_model("C:/Users/SomeName/Documents/GitHub/StealthGame/Game/Models/flat_vase");
    engine::vulkan::VulkanModel cube_model = engine.load_model("C:/Users/SomeName/Documents/GitHub/StealthGame/Game/Models/flat_vase");
    engine::vulkan::VulkanModel* models[2] = {&vase_model, &cube_model};
    initialize_world(world, ArrayRef{models, 2}, engine.get_aspect_ratio());
    setup_input_keyboard_system(world);
    setup_keyboard_movement(world);
    engine.run();
}
