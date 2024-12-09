#include "Engine.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include "../../Vendor/flecs/flecs.h"
#include "Components/Renderable.h"
#include "Components/Transform3D.h"
#include "Containers/DynArray.h"
#include "Systems/Systems.h"
#include "Vulkan/Camera.h"
#include "Vulkan/VulkanRenderInfo.h"

namespace global_arena {

constexpr size_t BIGGER_STACK_SIZE = 2 << 25;
static Arena s_ecs_arena{BIGGER_STACK_SIZE};

}

void* custom_malloc(ecs_size_t size) {
    return global_arena::s_ecs_arena.push(size);
}

void* custom_realloc(void* ptr, ecs_size_t size) {
    void* new_ptr;
    if (ptr == nullptr) {
        new_ptr = custom_malloc(size);
        ptr = new_ptr;
    } else {
        new_ptr = global_arena::s_ecs_arena.push(size);
    }
    if (ptr == new_ptr) {
        return ptr;
    }
    memcpy(new_ptr, ptr, size);
    return new_ptr;
}

void* custom_calloc(ecs_size_t size) {
    void* new_ptr = global_arena::s_ecs_arena.push_zero(size);
    return new_ptr;
}

void custom_free(void* ptr) {
    // Free doesn't do anything atm...
}

void* operator new(size_t size) {
    std::cout << "Allocated " << std::dec << size << " bytes\n";
    return malloc(size);
}

void operator delete(void* p) noexcept {
    //std::cout << "Deleted 0x" << std::hex << p << "\n";
    free(p);
}

void* operator new[](size_t size) {
    std::cout << "Allocated " << std::dec << size << " bytes\n";
    return malloc(size);
}

void operator delete[](void* p) noexcept {
    free(p);
}

namespace engine {

constexpr int default_stack_size = 2 << 20;

vulkan::VulkanModel make_cube(vulkan::DeviceWrapper* device, Arena& arena) {
    const ArrayRef<vulkan::VulkanModel::Vertex> vertices{{
        // left face (white)
          {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
          {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
          {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
          {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

          // right face (yellow)
          {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
          {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
          {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

          // top face (orange, remember y axis points down)
          {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
          {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
          {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
          {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

          // bottom face (red)
          {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
          {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
          {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
          {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

          // nose face (blue)
          {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
          {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
          {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
          {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

          // tail face (green)
          {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
          {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
          {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
          {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
    }, arena};
    return {device, vertices};
}

flecs::world setup_flecs() {
    // Setup the ECS
    ecs_os_set_api_defaults();
    ecs_os_api_t os_api = ecs_os_get_api();
    os_api.malloc_ = custom_malloc;
    os_api.free_ = custom_free;
    os_api.calloc_ = custom_calloc;
    os_api.realloc_ = custom_realloc;
    ecs_os_set_api(&os_api);

    return flecs::world{};
}

StealthEngine::StealthEngine() : m_temp_arena_(default_stack_size),
     m_permanent_arena_(default_stack_size),
    m_world_(setup_flecs()),
    m_vulkan_wrapper_(m_temp_arena_),
    m_renderer_(m_temp_arena_, m_permanent_arena_,
        &m_vulkan_wrapper_.window(), m_vulkan_wrapper_.device(), m_vulkan_wrapper_.surface()),
    m_pipeline_(m_temp_arena_, &m_renderer_, m_vulkan_wrapper_.device()),
    m_model_(make_cube(m_vulkan_wrapper_.device(), m_permanent_arena_)) {
    
}

void StealthEngine::run() {
    m_temp_arena_.clear();
    const flecs::entity cube_entity = m_world_.entity();
    cube_entity.set<components::Transform3D>({{0.f, 0.f, 2.5f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}})
        .set<components::Renderable>({&m_model_});
    m_world_.add<Camera>();
    systems::setup_render_system(m_world_);
    bool should_continue = true;
    components::Transform3D test_transform{{0.f, 0.f, 2.5f}, {0.f, 0.f, 0.f}, {1.f, 1.f, 1.f}};
    while (!m_vulkan_wrapper_.window().should_close() && should_continue) {
        m_vulkan_wrapper_.window().glfw_poll_events();
        if (const auto cmd_buffer = m_renderer_.begin_frame(m_temp_arena_, m_permanent_arena_)) {
            m_renderer_.begin_render_pass(cmd_buffer);
            m_renderer_.bind_pipeline(m_pipeline_.get_pipeline());
            m_world_.set<VulkanRenderInfo>({cmd_buffer, m_pipeline_.get_pipeline_layout()});
            should_continue = m_world_.progress();
            m_renderer_.end_render_pass(cmd_buffer);
            m_renderer_.end_frame(m_temp_arena_, m_permanent_arena_);
        }
    }
}

ArrayRef<char> StealthEngine::read_temporary_file(Arena& temp_arena, const char* file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << file_name << "\n" << std::flush;
        throw std::runtime_error("Failed to open file");
    }
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t bytes_needed = sizeof(char) * file_size;
    const auto ptr = static_cast<char*>(temp_arena.push(bytes_needed));
    ArrayRef buffer{ptr, static_cast<size_t>(file_size)};
    file.read(ptr, file_size);
    file.close();
    return buffer;
}


}