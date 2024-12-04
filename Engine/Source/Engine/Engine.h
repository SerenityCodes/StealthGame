#pragma once

#include "../../Vendor/entt/include/entt.hpp"
#include "Vulkan/BasicRenderer.h"
#include "Vulkan/VulkanWrapper.h"
#include "Vulkan/Wrappers/PipelineWrapper.h"

namespace engine {

	class StealthEngine {
	    entt::registry m_registry_;
	    vulkan::VulkanWrapper m_vulkan_wrapper_;
	    vulkan::BasicRenderer m_renderer_;
	    vulkan::PipelineWrapper m_pipeline_;
	    std::unique_ptr<vulkan::VulkanModel> m_model_;
	public:
	    StealthEngine();
	    ~StealthEngine() = default;

	    void run();

	    static DynArray<char> read_file(const char* file_name);
	};

}