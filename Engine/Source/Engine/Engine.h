#pragma once

#include "../../Vendor/entt/include/entt.hpp"
#include "Vulkan/BasicRenderer.h"
#include "Vulkan/VulkanWrapper.h"
#include "Vulkan/Wrappers/PipelineWrapper.h"

namespace engine {
	class StealthEngine {
	    entt::basic_registry<entt::entity, allocators::StackAllocator<entt::entity>> m_registry_;
	    allocators::StackAllocator<void> m_temp_allocator_;
	    allocators::StackAllocator<void> m_vulkan_allocator_;
	    vulkan::VulkanWrapper m_vulkan_wrapper_;
	    vulkan::BasicRenderer m_renderer_;
	    vulkan::PipelineWrapper m_pipeline_;
	    std::unique_ptr<vulkan::VulkanModel> m_model_;

	    using CharacterAllocator = allocators::StackAllocator<void>::rebind<char>::other;
	public:
	    StealthEngine();
	    ~StealthEngine() = default;

	    void run();

	    static DynArray<char> read_file(const char* file_name);
	    static std::vector<char, allocators::StackAllocator<char>> read_temporary_file(const char* file_name, const allocators::StackAllocator<void>& allocator);
	};

}