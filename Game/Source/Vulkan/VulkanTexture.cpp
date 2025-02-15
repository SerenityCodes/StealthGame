#include "VulkanTexture.h"

#include <cassert>
#include <stdexcept>

#include "common.h"
#include "../../../Vendor/stb_image/stb_image.h"

namespace engine::vulkan {

void VulkanTexture::create_texture_image_view(VkFormat format) {
    VkImageViewCreateInfo view_info{};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = m_image_;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VULKAN_ASSERT(vkCreateImageView(m_device_, &view_info, nullptr, &m_image_view_), "Failed to create image view!")
}

void VulkanTexture::create_texture_sampler(VulkanRenderer& renderer) {
    VkSamplerCreateInfo sampler_info{};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(renderer.vulkan_physical_device(), &properties);
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    
    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;

    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.mipLodBias = 0.0f;

    VULKAN_ASSERT(vkCreateSampler(m_device_, &sampler_info, nullptr, &m_sampler_), "Failed to create image sampler")
}

VulkanTexture::VulkanTexture(VulkanRenderer& renderer, const char* file_path) : m_device_(renderer.vulkan_device()), m_allocator_(renderer.vma_allocator()) {
    stbi_uc* pixels = stbi_load(file_path, &m_width_, &m_height_, &m_channels_, STBI_rgb_alpha);
    VkDeviceSize size = m_width_ * m_height_;

    ENGINE_ASSERT(pixels, "Failed to load texture!")
    
    m_buffer_ = DeviceBufferWrapper{renderer, sizeof(int), static_cast<uint32_t>(size), VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT};
    DeviceBufferWrapper staging_buffer{renderer,
        sizeof(int),
        static_cast<uint32_t>(size),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT};
    staging_buffer.write_to_buffer(pixels);
    renderer.copy_buffer(staging_buffer.buffer(), m_buffer_.buffer(), size);
    stbi_image_free(pixels);

    VkImageCreateInfo image_create_info;
    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = m_width_;
    image_create_info.extent.height = m_height_;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.flags = 0;

    VmaAllocationCreateInfo vma_create_info;
    vma_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    vma_create_info.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    vma_create_info.priority = 1.0f;
    
    VULKAN_ASSERT(vmaCreateImage(renderer.vma_allocator(), &image_create_info, &vma_create_info, &m_image_, &m_allocation_, nullptr), "Failed to create image")

    transition_image_layout(renderer, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(renderer, m_buffer_, m_width_, m_height_);
}

VulkanTexture::~VulkanTexture() {
    vkDestroySampler(m_device_, m_sampler_, nullptr);
    vkDestroyImageView(m_device_, m_image_view_, nullptr);
    vmaFreeMemory(m_allocator_, m_allocation_);
}

VkImage VulkanTexture::get_image() const {
    return m_image_;
}

void VulkanTexture::transition_image_layout(VulkanRenderer& renderer,
    VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout) const {
    VkCommandBuffer command_buffer = renderer.get_one_time_cmd_buffer();

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image_;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    
    barrier.srcAccessMask = 0; // TODO: Not supposed to be 0. Fix this
    barrier.dstAccessMask = 0; // TODO: Not supposed to be 0. Fix this

    VkPipelineStageFlags src_stage = 0;
    VkPipelineStageFlags dst_stage = 0;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dst_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        src_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dst_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        assert(false);
    }
    
    vkCmdPipelineBarrier(command_buffer, src_stage, dst_stage,
        0,
        0,
        nullptr,
        0,
        nullptr,
        1,
        &barrier);

    renderer.end_one_time_cmd_buffer(command_buffer);
}

void VulkanTexture::copy_buffer_to_image(VulkanRenderer& renderer, DeviceBufferWrapper& buffer, u32 width, u32 height) const {
    VkCommandBuffer command_buffer = renderer.get_one_time_cmd_buffer();

    VkBufferImageCopy copy_region;
    copy_region.bufferOffset = 0;
    copy_region.bufferRowLength = 0;
    copy_region.bufferImageHeight = 0;

    copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    copy_region.imageSubresource.mipLevel = 0;
    copy_region.imageSubresource.baseArrayLayer = 0;
    copy_region.imageSubresource.layerCount = 1;

    copy_region.imageOffset = {0, 0, 0};
    copy_region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer.buffer(), m_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    
    renderer.end_one_time_cmd_buffer(command_buffer);
}

}