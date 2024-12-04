#include "Engine.h"

#include <fstream>
#include <iostream>

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

StealthEngine::StealthEngine() : frame_allocator_(default_stack_size), m_renderer_(&m_vulkan_wrapper_.window(), m_vulkan_wrapper_.device(), m_vulkan_wrapper_.surface()), m_pipeline_(&m_renderer_, m_vulkan_wrapper_.device()) { }

void StealthEngine::run() {
    while (!m_vulkan_wrapper_.window().should_close()) {
        m_vulkan_wrapper_.window().glfw_poll_events();
        if (auto cmd_buffer = m_renderer_.begin_frame()) {
            m_renderer_.begin_render_pass(cmd_buffer);
            m_pipeline_.bind(cmd_buffer);
            m_pipeline_.draw(cmd_buffer);
            // Add code to draw objects here
            m_renderer_.end_render_pass(cmd_buffer);
            m_renderer_.end_frame();
        }
    }
}

DynArray<char> StealthEngine::read_file(const char* file_name) {
    std::ifstream file(file_name, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Failed to open file " << file_name << "\n" << std::flush;
        throw std::runtime_error("Failed to open file");
    }
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    DynArray<char> buffer(file_size);
    file.read(buffer.data(), file_size);
    file.close();
    return buffer;
}


}