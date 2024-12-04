#pragma once

#include <glslang/Include/glslang_c_shader_types.h>
#include <glslang/Public/ShaderLang.h>

#include "Allocators/StackAllocator.h"
#include "Vulkan/VulkanWrapper.h"

namespace engine {

	class StealthEngine {
	    allocators::StackAllocator frame_allocator_;
	    vulkan::VulkanWrapper m_vulkan_wrapper_;
	    vulkan::BasicRenderer m_renderer_;
	    vulkan::PipelineWrapper m_pipeline_;
	public:
	    StealthEngine();
	    ~StealthEngine() = default;

	    void run();

	    static DynArray<char> read_file(const char* file_name);
	};

}