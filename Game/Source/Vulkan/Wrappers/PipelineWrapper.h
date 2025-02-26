#pragma once

#include <glm/glm.hpp>

#include "Shader.h"
#include "SwapChain.h"
#include "common.h"
#include "Vulkan/ShaderEnum.h"
#include "Vulkan/VulkanModel.h"
#include "Vulkan/VulkanRenderer.h"

namespace engine::vulkan {

constexpr VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

class PipelineWrapper {
public:
    struct DescriptorSetLayoutData {
        u32 set_number;
        VkDescriptorSetLayoutCreateInfo create_info;
        DynArray<VkDescriptorSetLayoutBinding> bindings;
    };
private:
    VkDevice m_device_;
    DynArray<VkShaderModule> m_shaders_;
    VkPipelineLayout m_pipeline_layout_;
    VkPipeline m_pipeline_;

    VkShaderModule load_shader_code(ShaderEnum shader_to_load, Arena& temp_arena) const;
public:
    PipelineWrapper(Arena& temp_arena, VulkanRenderer& renderer);
    ~PipelineWrapper();
    
    VkPipelineLayout get_pipeline_layout() const;
    VkPipeline get_pipeline() const;

    void bind(VkCommandBuffer command_buffer) const;
    
    static VkShaderModule create_shader_module(VkDevice device, const DynArray<byte>& code);
};

}
