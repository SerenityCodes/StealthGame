#include "Engine.h"

#include <filesystem>
#include <fstream>
#include <iostream>

#include "common.h"
#include "imgui.h"
#include "Systems/CoreEngineSystems.h"
#include "Vulkan/VulkanRenderInfo.h"
#include "Logging/Logger.h"
#include "Vulkan/Renderer.h"

namespace engine {

constexpr int default_stack_size = 2 << 25;

StealthEngine::StealthEngine() : m_temp_arena_((Logger::Init(), default_stack_size / 2)),
     m_permanent_arena_(default_stack_size)
    //m_renderer_(1200, 800, m_temp_arena_, m_permanent_arena_)
    {
    //m_pipeline_.emplace(m_temp_arena_, m_renderer_);
}

void StealthEngine::run() {
    ENGINE_LOG_INFO("Engine starting...")
    Renderer renderer;
    renderer.render();
}
    // Join Threads Here

flecs::world& StealthEngine::get_world() {
    return m_world_;
}

float StealthEngine::get_aspect_ratio() const {
    return 0.0f;
}


}