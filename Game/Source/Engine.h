#pragma once

#include "Containers/ArrayRef.h"
#include "Memory/Arena.h"
#include "Vulkan/Wrappers/PipelineWrapper.h"
#include "../Vendor/flecs/flecs.h"
#include "Containers/String.h"
#include "Vulkan/VulkanRenderer.h"

namespace engine {
	class StealthEngine {
	    Arena m_temp_arena_;
	    Arena m_permanent_arena_;
	    vulkan::VulkanRenderer m_renderer_;
	    vulkan::PipelineWrapper m_pipeline_;
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
	    vulkan::VulkanModel create_model(const vulkan::VulkanModel::VertexIndexInfo& index_info);
	    vulkan::VulkanModel load_model(const String& file_name);
	    vulkan::VulkanModel load_model(const char* file_name);
	    float get_aspect_ratio() const;

	    static ArrayRef<byte> read_temporary_file(Arena& temp_arena, const String& file_name);
	    static ArrayRef<byte> read_temporary_file(Arena& temp_arena, const char* file_name);
	};

}