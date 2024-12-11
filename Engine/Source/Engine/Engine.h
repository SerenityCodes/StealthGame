#pragma once

#include "Containers/ArrayRef.h"
#include "Memory/Arena.h"
#include "Vulkan/BasicRenderer.h"
#include "Vulkan/VulkanWrapper.h"
#include "Vulkan/Wrappers/PipelineWrapper.h"

namespace engine {
	class StealthEngine {
	    Arena m_temp_arena_;
	    Arena m_permanent_arena_;
	    vulkan::VulkanWrapper m_vulkan_wrapper_;
	    vulkan::BasicRenderer m_renderer_;
	    vulkan::PipelineWrapper m_pipeline_;
	    vulkan::VulkanModel m_model_;
	public:
	    StealthEngine();
	    StealthEngine(const StealthEngine&) = delete;
	    StealthEngine(StealthEngine&&) = delete;
	    StealthEngine& operator=(const StealthEngine&) = delete;
	    StealthEngine& operator=(StealthEngine&&) = delete;
	    ~StealthEngine() = default;

	    void run();

	    static ArrayRef<char> read_temporary_file(Arena& temp_arena, const char* file_name);
	};

}