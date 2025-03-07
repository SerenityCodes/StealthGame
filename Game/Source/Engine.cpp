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
    {
}

void StealthEngine::run() {
    ENGINE_LOG_INFO("Engine starting...")
    Renderer renderer;
    renderer.render();
    // Join Threads Here
}

flecs::world& StealthEngine::get_world() {
    return m_world_;
}


}