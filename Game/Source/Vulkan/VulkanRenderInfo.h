#pragma once

#include <vulkan/vulkan.h>

struct VulkanRenderInfo {
    VkCommandBuffer cmd_buffer;
    VkPipelineLayout pipeline_layout;
};
