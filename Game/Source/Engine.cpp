#include "Engine.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "common.h"
#include "imgui.h"
#include "Systems/CoreEngineSystems.h"
#include "Vulkan/VulkanRenderInfo.h"
#include "Logging/Logger.h"

void* operator new(size_t size) {
    std::cout << "Allocated bytes: " << size << "\n";
    return malloc(size);
}

void operator delete(void* p) noexcept {
    //std::cout << "Deleted 0x" << std::hex << p << "\n";
    free(p);
}

void* operator new[](size_t size) {
    std::cout << "Allocated bytes: " << size << "\n";
    return malloc(size);
}

void operator delete[](void* p) noexcept {
    free(p);
} 

namespace engine {

constexpr int default_stack_size = 2 << 25;

StealthEngine::StealthEngine() : m_temp_arena_((Logger::Init(), default_stack_size / 2)),
     m_permanent_arena_(default_stack_size),
    m_renderer_(1200, 800, m_temp_arena_, m_permanent_arena_)
    {
    m_pipeline_.emplace(m_temp_arena_, m_renderer_);
}

void StealthEngine::run() {
    ENGINE_LOG_INFO("Engine starting...")
    m_world_.add<VulkanRenderInfo>();
    m_world_.set<components::WindowComponent>({m_renderer_.raw_glfw_window()});
    systems::setup_render_system(m_world_);
    bool should_continue = true;
    while (!m_renderer_.should_close_window() && should_continue) {
        m_renderer_.poll();
        m_temp_arena_.clear();
        if (const auto cmd_buffer = m_renderer_.begin_frame(m_temp_arena_)) {
            m_renderer_.begin_render_pass(cmd_buffer, vulkan::VulkanRenderer::DEFAULT);
            m_renderer_.bind_pipeline(m_pipeline_->get_pipeline());
            VulkanRenderInfo* render_info = m_world_.get_mut<VulkanRenderInfo>();
            render_info->cmd_buffer = cmd_buffer;
            render_info->pipeline_layout = m_pipeline_->get_pipeline_layout();
            should_continue = m_world_.progress();
            vulkan::VulkanRenderer::end_render_pass(cmd_buffer);
            m_renderer_.end_frame(m_temp_arena_);
#ifdef IMGUI_ENABLED
            m_renderer_.render_imgui(m_temp_arena_);
#endif
        }
    }
    // Join Threads Here
}

flecs::world& StealthEngine::get_world() {
    return m_world_;
}

vulkan::VulkanModel StealthEngine::create_model(vulkan::VulkanModel::VertexIndexInfo& index_info) {
    return vulkan::VulkanModel{m_renderer_, index_info};
}

vulkan::VulkanModel StealthEngine::load_model(const arena_string& file_name) {
    return vulkan::VulkanModel::load_model(m_temp_arena_, m_permanent_arena_, m_renderer_, file_name);
}

vulkan::VulkanModel StealthEngine::load_model(const char* file_name) {
    return vulkan::VulkanModel::load_model(m_temp_arena_, m_permanent_arena_, m_renderer_, arena_string{file_name, STLArenaAllocator<char>{&m_temp_arena_}});
}

float StealthEngine::get_aspect_ratio() const {
    return m_renderer_.get_aspect_ratio();
}


}