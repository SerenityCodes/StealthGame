#pragma once

#include "Containers/ArrayRef.h"
#include "MemoryArena/Arena.h"
#include "Vulkan/BasicRenderer.h"
#include "Vulkan/VulkanWrapper.h"
#include "Vulkan/Wrappers/PipelineWrapper.h"

#include "../Vendor/flecs/flecs.h"

namespace engine {
	class StealthEngine {
	    Arena m_temp_arena_;
	    Arena m_permanent_arena_;
	    flecs::world m_world_;
	    vulkan::VulkanWrapper m_vulkan_wrapper_;
	    vulkan::BasicRenderer m_renderer_;
	    vulkan::PipelineWrapper m_pipeline_;
	    vulkan::VulkanModel m_model_;
	public:
	    StealthEngine();
	    ~StealthEngine() = default;

	    void run();

	    static ArrayRef<char> read_temporary_file(Arena& temp_arena, const char* file_name);
	};

}