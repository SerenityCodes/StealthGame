#pragma once

#include "Memory/Arena.h"
#include "Vulkan/Wrappers/PipelineWrapper.h"
#include "../Vendor/flecs/flecs.h"
#include "Vulkan/VulkanRenderer.h"

namespace engine {
	class StealthEngine {
	    Arena m_temp_arena_;
	    Arena m_permanent_arena_;
	    vulkan::VulkanRenderer m_renderer_;
	    ObjectHolder<vulkan::PipelineWrapper> m_pipeline_;
	    flecs::world m_world_;
	public:
	    StealthEngine();
	    StealthEngine(const StealthEngine&) = delete;
	    StealthEngine(StealthEngine&&) = delete;
	    StealthEngine& operator=(const StealthEngine&) = delete;
	    StealthEngine& operator=(StealthEngine&&) = delete;
	    ~StealthEngine() = default;

	    void run();
	    flecs::world& get_world();
	    vulkan::VulkanModel create_model(vulkan::VulkanModel::VertexIndexInfo& index_info);
	    vulkan::VulkanModel load_model(const arena_string& file_name);
	    vulkan::VulkanModel load_model(const char* file_name);
	    float get_aspect_ratio() const;
	};

}