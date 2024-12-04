#pragma once

#include "DeviceWrapper.h"
#include "SwapChain.h"
#include "Engine/Vulkan/BasicRenderer.h"

namespace engine::vulkan {

constexpr VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

class PipelineWrapper {
    DeviceWrapper* m_device_;
    VkShaderModule m_vertex_shader_;
    VkShaderModule m_fragment_shader_;
    VkPipelineLayout m_pipeline_layout_;
    VkPipeline m_pipeline_;
public:
    PipelineWrapper(BasicRenderer* renderer, DeviceWrapper* device);
    ~PipelineWrapper();
    
    VkPipelineLayout get_pipeline_layout() const;
    VkPipeline get_pipeline() const;
    VkShaderModule get_vertex_shader() const;
    VkShaderModule get_fragment_shader() const;

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);
    
    static VkShaderModule create_shader_module(VkDevice device, const DynArray<char>& code);
};

}
