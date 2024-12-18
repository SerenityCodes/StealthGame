#include <random>

#include "Engine/Engine.h"
#include "Engine/Systems/CoreEngineSystems.h"
#include "Engine/Vulkan/Camera.h"

struct Velocity {
    glm::vec3 direction;
    float speed;
};

engine::vulkan::VulkanModel make_cube(engine::StealthEngine& engine, Arena& arena) {
    engine::vulkan::VulkanModel::VertexIndexInfo index_info;
    index_info.vertices = {{
        // left face (white)
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
 
        // right face (yellow)
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
 
        // top face (orange, remember y axis points down)
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
 
        // bottom face (red)
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
 
        // nose face (blue)
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
 
        // tail face (green)
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
    }, arena};
    index_info.indices = {{0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                            12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21}, arena};
    return engine.create_model(index_info);
}

glm::vec3 get_random_direction() {
    static std::mt19937_64 gen{std::random_device{}()};
    static std::uniform_real_distribution dist(-1.f, 1.f);

    glm::vec3 direction = glm::vec3(dist(gen), dist(gen), dist(gen));
    if (glm::length(direction) > 0.0f) {
        direction = glm::normalize(direction);
    }
    return direction;
}

void spawn_and_move_cube(const flecs::world& world) {
    world.system<components::Transform3D, Velocity>()
        .kind(flecs::OnUpdate)
        .each([](flecs::entity entity, components::Transform3D& transform, Velocity& velocity) {
            static float timer = 0.0f;
            static int cube_count = 1;
            constexpr float rotation_speed = 0.5f;
            constexpr float fall_speed = 0.01f;
            float delta_time = entity.world().delta_time();
            timer += delta_time;
            transform.rotation.x = glm::mod(transform.rotation.x + delta_time * rotation_speed, glm::two_pi<float>());
            transform.rotation.y = glm::mod(transform.rotation.y + delta_time * rotation_speed * 2, glm::two_pi<float>());

            // Send it in a random direction
            transform.translation += velocity.speed * velocity.direction * delta_time;

            if (timer >= 3.0f) {
                timer = 0.0f;
                auto world = entity.world();
                flecs::entity cube = world.entity();
                cube.set<components::Transform3D>({.translation  = {0.f, 0.f, 2.5f}, .rotation = {0.f, 0.f, 0.f}, .scale = {.5f, .5f, .5f}});
                cube.set<components::Renderable>({entity.get<components::Renderable>()->model});
                cube.set<Velocity>({.direction = get_random_direction(), .speed = .5f});
                cube_count++;
            }
           if (transform.translation.x > 1.0f || transform.translation.x < -1.0f || transform.translation.y > 1.0f || transform.translation.y < -1.0f) {
               entity.destruct();
           } 
        });
}

void initialize_world(const flecs::world& world, engine::vulkan::VulkanModel* model, float aspect) {
    world.emplace<Camera>(glm::radians(45.0f), aspect, 0.1f, 10.f);
    flecs::entity cube = world.entity();
    cube.set<components::Transform3D>({.translation  = {0.f, 0.f, 2.5f}, .rotation = {0.f, 0.f, 0.f}, .scale = {.5f, .5f, .5f}});
    cube.set<components::Renderable>({model});
    cube.set<Velocity>({.direction = get_random_direction(), .speed = .5f});
    spawn_and_move_cube(world);
}

int main() {
    Arena cube_arena{2 << 20};
	engine::StealthEngine engine;
    flecs::world& world = engine.get_world();
    engine::vulkan::VulkanModel cube_model = make_cube(engine, cube_arena);
    initialize_world(world, &cube_model, engine.get_aspect_ratio());
    engine.run();
}
