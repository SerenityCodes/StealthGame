#include "Engine.h"

#include <filesystem>
#include <fstream>

#include "common.h"
#include "imgui.h"
#include "Logging/Logger.h"
#include "Rendering/Renderer.h"

namespace engine {

constexpr int default_stack_size = 2 << 25;

StealthEngine::StealthEngine() : m_temp_arena_((Logger::Init(), default_stack_size / 2)),
     m_permanent_arena_(default_stack_size)
    {
}

void StealthEngine::run() {
    ENGINE_LOG_INFO("Engine starting...")
    Renderer renderer{m_world_};
    renderer.render();
}

flecs::world& StealthEngine::get_world() {
    return m_world_;
}


}