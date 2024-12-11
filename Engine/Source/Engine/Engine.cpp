#include "Engine.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include "Components/Renderable.h"
#include "Components/Transform3D.h"
#include "Containers/DynArray.h"
#include "Systems/Systems.h"
#include "Vulkan/Camera.h"
#include "Vulkan/VulkanRenderInfo.h"

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

StealthEngine::StealthEngine() : m_temp_arena_(default_stack_size),
     m_permanent_arena_(default_stack_size),
    m_vulkan_wrapper_(m_temp_arena_),
    m_renderer_(m_temp_arena_, m_permanent_arena_,
        &m_vulkan_wrapper_.window(), m_vulkan_wrapper_.device(), m_vulkan_wrapper_.surface()),
    m_pipeline_(m_temp_arena_, &m_renderer_, m_vulkan_wrapper_.device()),
    m_model_(make_cube(m_vulkan_wrapper_.device(), m_permanent_arena_)) {
    
}

void StealthEngine::run() {
    m_temp_arena_.clear();
    bool should_continue = true;
    components::Transform3D test_transform{{0.f, 0.f, 2.5f}, {0.f, 0.f, 0.f}, {.5f, .5f, .5f}};
    components::Renderable test_renderable{.model = &m_model_};
    Camera camera;
    camera.set_perspective_projection(glm::radians(45.0f), m_renderer_.get_aspect_ratio(), 0.1f, 10.f);
    auto start = std::chrono::high_resolution_clock::now();
    while (!m_vulkan_wrapper_.window().should_close() && should_continue) {
        m_vulkan_wrapper_.window().glfw_poll_events();
        auto new_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(new_time - start).count();
        start = new_time;
        if (const auto cmd_buffer = m_renderer_.begin_frame(m_temp_arena_, m_permanent_arena_)) {
            m_renderer_.begin_render_pass(cmd_buffer);
            m_renderer_.bind_pipeline(m_pipeline_.get_pipeline());
            VulkanRenderInfo render_info{.cmd_buffer = cmd_buffer, .pipeline_layout = m_pipeline_.get_pipeline_layout()};
            systems::render_system(render_info, test_transform, test_renderable, delta_time, camera);
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