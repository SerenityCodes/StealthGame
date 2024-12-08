#include "Engine.h"

#include <fstream>
#include <iostream>

#include "Components/Transform3D.h"
#include "Containers/DynArray.h"

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

std::unique_ptr<vulkan::VulkanModel> make_cube(vulkan::DeviceWrapper* device, glm::vec3 offset) {
    ArrayRef<vulkan::VulkanModel::Vertex> vertices{
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
    };
    for (auto& v : vertices) {
        v.position += offset;
    }
    return std::make_unique<vulkan::VulkanModel>(device, std::move(vertices));
}

StealthEngine::StealthEngine() : m_temp_arena_(default_stack_size),
                                 m_permanent_arena_(default_stack_size), m_vulkan_wrapper_(m_temp_arena_), m_renderer_(m_temp_arena_, m_permanent_arena_, &m_vulkan_wrapper_.window(), m_vulkan_wrapper_.device(), m_vulkan_wrapper_.surface()), m_pipeline_(m_temp_arena_, &m_renderer_, m_vulkan_wrapper_.device()), m_model_(make_cube(m_vulkan_wrapper_.device(), {0, 0, 0})) { }

void StealthEngine::run() {
    while (!m_vulkan_wrapper_.window().should_close()) {
        m_vulkan_wrapper_.window().glfw_poll_events();
        if (auto cmd_buffer = m_renderer_.begin_frame(m_temp_arena_, m_permanent_arena_)) {
            m_renderer_.begin_render_pass(cmd_buffer);
            m_pipeline_.bind(cmd_buffer);
            components::Transform3D transform{{1.f, 1.f, 1.f}, {1., 1., 1.}, {2, 2, 2} };
            vulkan::PipelineWrapper::SimplePushConstantData push_data{};
            push_data.transform = transform.as_matrix();
            vkCmdPushConstants(cmd_buffer, m_pipeline_.get_pipeline_layout(), VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(vulkan::PipelineWrapper::SimplePushConstantData), &push_data);
            m_model_->bind(cmd_buffer);
            m_model_->draw(cmd_buffer);
            // Add code to draw objects here
            m_renderer_.end_render_pass(cmd_buffer);
            m_renderer_.end_frame(m_temp_arena_, m_permanent_arena_);
        }
    }
}

ArrayRef<char> StealthEngine::read_temporary_file(Arena& temp_file, const char* file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << file_name << "\n" << std::flush;
        throw std::runtime_error("Failed to open file");
    }
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t bytes_needed = sizeof(char) * file_size;
    const auto ptr = static_cast<char*>(temp_file.push(bytes_needed));
    ArrayRef buffer{ptr, static_cast<size_t>(file_size)};
    file.read(ptr, file_size);
    file.close();
    return buffer;
}


}