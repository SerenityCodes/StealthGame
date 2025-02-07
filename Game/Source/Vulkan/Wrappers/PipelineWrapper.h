﻿#pragma once

#include <glm/glm.hpp>

#include "DeviceWrapper.h"
#include "SwapChain.h"
#include "common.h"
#include "Vulkan/BasicRenderer.h"
#include "Vulkan/VulkanModel.h"

namespace engine::vulkan {

constexpr VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

class PipelineWrapper {
public:
    struct SimplePushConstantData {
        glm::mat4 transform;
        glm::mat4 normal;
    };
private:
    DeviceWrapper* m_device_;
    VkShaderModule m_vertex_shader_;
    VkShaderModule m_fragment_shader_;
    VkPipelineLayout m_pipeline_layout_;
    VkPipeline m_pipeline_;
public:
    PipelineWrapper(Arena& temp_arena, BasicRenderer* renderer, DeviceWrapper* device);
    ~PipelineWrapper();
    
    VkPipelineLayout get_pipeline_layout() const;
    VkPipeline get_pipeline() const;
    VkShaderModule get_vertex_shader() const;
    VkShaderModule get_fragment_shader() const;

    void bind(VkCommandBuffer command_buffer) const;
    
    static VkShaderModule create_shader_module(VkDevice device, const ArrayRef<byte>& code);
};

}
