#include "Engine.h"

#include <chrono>
#include <fstream>
#include <iostream>

#include "Systems/CoreEngineSystems.h"
#include "Vulkan/VulkanRenderInfo.h"
#include "Logging/Logger.h"

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

constexpr int default_stack_size = 2 << 25;

StealthEngine::StealthEngine() : m_temp_arena_(default_stack_size),
     m_permanent_arena_(default_stack_size),
    m_vulkan_wrapper_(m_temp_arena_),
    m_renderer_(m_temp_arena_, m_permanent_arena_,
    &m_vulkan_wrapper_.window(), m_vulkan_wrapper_.device(), m_vulkan_wrapper_.surface()),
    m_pipeline_(m_temp_arena_, &m_renderer_, m_vulkan_wrapper_.device())
    {
    Logger::Init();
}

void StealthEngine::run() {
    ENGINE_LOG_INFO("Starting run engine...")
    m_world_.add<VulkanRenderInfo>();
    m_world_.set<components::WindowComponent>({m_vulkan_wrapper_.window().raw_window()});
    systems::setup_render_system(m_world_);
    bool should_continue = true;
    while (!m_vulkan_wrapper_.window().should_close() && should_continue) {
        m_vulkan_wrapper_.window().glfw_poll_events();
        m_temp_arena_.clear();
        if (const auto cmd_buffer = m_renderer_.begin_frame(m_temp_arena_, m_permanent_arena_)) {
            m_renderer_.begin_render_pass(cmd_buffer);
            m_renderer_.bind_pipeline(m_pipeline_.get_pipeline());
            VulkanRenderInfo* render_info = m_world_.get_mut<VulkanRenderInfo>();
            render_info->cmd_buffer = cmd_buffer;
            render_info->pipeline_layout = m_pipeline_.get_pipeline_layout();
            should_continue = m_world_.progress();
            m_renderer_.end_render_pass(cmd_buffer);
            m_renderer_.end_frame(m_temp_arena_, m_permanent_arena_);
        }
    }
    // Join Threads Here
}

flecs::world& StealthEngine::get_world() {
    return m_world_;
}

vulkan::VulkanModel StealthEngine::create_model(
    const vulkan::VulkanModel::VertexIndexInfo& index_info) {
    return {m_vulkan_wrapper_.device(), m_renderer_.get_command_pool(), index_info};
}

vulkan::VulkanModel StealthEngine::load_model(const char* file_name, uint32_t import_flags) {
    return vulkan::VulkanModel::load_model(m_temp_arena_, m_permanent_arena_, m_vulkan_wrapper_.device(), m_renderer_.get_command_pool(), file_name, import_flags);
}

float StealthEngine::get_aspect_ratio() const {
    return m_renderer_.get_aspect_ratio();
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