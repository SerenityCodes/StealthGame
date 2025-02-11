#include "DescriptorPool.h"

#include "common.h"
#include "imgui_impl_vulkan.h"

namespace engine::vulkan {

DescriptorPool::DescriptorPool(DeviceWrapper& device) : m_device_(&device) {
    VkDescriptorPoolSize pool_sizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE}
    };
    VkDescriptorPoolCreateInfo pool_create_info{};
    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_create_info.pPoolSizes = pool_sizes;
    pool_create_info.poolSizeCount = 1;
    pool_create_info.maxSets = 0;
    for (auto& [type, descriptorCount] : pool_sizes) {
        pool_create_info.maxSets += descriptorCount;
    }
    ENGINE_ASSERT(vkCreateDescriptorPool(*m_device_, &pool_create_info, nullptr, &m_descriptor_pool_) == VK_SUCCESS, "Failed to create descriptor pool!")
}

DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(*m_device_, m_descriptor_pool_, nullptr);
}

VkDescriptorPool DescriptorPool::descriptor_pool() const {
    return m_descriptor_pool_;
}

}