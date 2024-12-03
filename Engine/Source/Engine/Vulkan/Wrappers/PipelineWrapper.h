#pragma once

#include "DeviceWrapper.h"
#include "SwapChain.h"

namespace engine::vulkan {

constexpr VkDynamicState DYNAMIC_STATES[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

class PipelineWrapper {
    DeviceWrapper* m_device_;
    SwapChain* m_swap_chain_;
    VkShaderModule m_vertex_shader_;
    VkShaderModule m_fragment_shader_;
    VkPipelineLayout m_pipeline_layout_;
    VkRenderPass m_render_pass_;
    VkPipeline m_pipeline_;
public:
    PipelineWrapper(SwapChain* swap_chain, DeviceWrapper* device);
    ~PipelineWrapper();

    VkRenderPass create_render_pass() const;

    VkPipelineLayout get_pipeline_layout() const;
    VkPipeline get_pipeline() const;
    VkShaderModule get_vertex_shader() const;
    VkShaderModule get_fragment_shader() const;
    VkRenderPass get_render_pass() const;

    void set_new_swap_chain_ptr(SwapChain* swap_chain);

    static VkShaderModule create_shader_module(VkDevice device, const DynArray<char>& code);
};

}
