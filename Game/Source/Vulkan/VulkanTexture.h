#pragma once

#include "Wrappers/DeviceBufferWrapper.h"

#include <string>

namespace engine::vulkan {

class VulkanTexture {
    VkDevice m_device_;
    VkImage m_image_;
    VkImageView m_image_view_;
    VkSampler m_sampler_;
    DeviceBufferWrapper m_buffer_;
    int m_width_{0}, m_height_{0}, m_channels_{0};

    VmaAllocator m_allocator_;
    VmaAllocation m_allocation_;

    void create_texture_image_view(VkFormat format);
    void create_texture_sampler(VulkanRenderer& renderer);
public:
    VulkanTexture(VulkanRenderer& renderer, const char* file_path);
    ~VulkanTexture();

    VkImage get_image() const;
    void transition_image_layout(VulkanRenderer& renderer, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) const;
    void copy_buffer_to_image(VulkanRenderer& renderer, DeviceBufferWrapper& buffer, uint32_t width, uint32_t height) const;
};

}
