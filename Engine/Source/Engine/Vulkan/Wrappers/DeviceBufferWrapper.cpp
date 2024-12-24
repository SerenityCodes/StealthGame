#include <Engine/Vulkan/Wrappers/DeviceBufferWrapper.h>

#include "../../../../Vendor/vma/include/vk_mem_alloc.h"

namespace engine::vulkan {

VkDeviceSize DeviceBufferWrapper::aligned_size(VkDeviceSize size,
    VkDeviceSize alignment) {
    if (alignment > 0) {
        return (size + alignment - 1) & ~(alignment - 1);
    }
    return size;
}

DeviceBufferWrapper::DeviceBufferWrapper(
    DeviceWrapper* device_wrapper, VkDeviceSize instance_size,
    uint32_t instance_count, VkBufferUsageFlags flags,
    VmaAllocationCreateFlags allocation_flags,
    VkDeviceSize min_offset_alignment)
    : m_device_wrapper_(device_wrapper),
    m_alignment_size_(aligned_size(instance_size, min_offset_alignment)),
    m_buffer_size_(m_alignment_size_ * instance_count),
    m_instance_size_(instance_size) {
    device_wrapper->create_buffer(m_buffer_size_, flags, allocation_flags, &m_buffer_, &m_allocation_);
}

DeviceBufferWrapper::DeviceBufferWrapper(DeviceBufferWrapper&& other) noexcept {
    m_device_wrapper_ = other.m_device_wrapper_;
    m_buffer_ = other.m_buffer_;
    m_buffer_size_ = other.m_buffer_size_;
    m_instance_size_ = other.m_instance_size_;
    m_allocation_ = other.m_allocation_;
    m_alignment_size_ = other.m_alignment_size_;
    m_instance_size_ = other.m_instance_size_;
    other.m_device_wrapper_ = nullptr;
}

DeviceBufferWrapper& DeviceBufferWrapper::operator=(
    DeviceBufferWrapper&& other) noexcept {
    if (this != &other) {
        m_device_wrapper_ = other.m_device_wrapper_;
        m_buffer_ = other.m_buffer_;
        m_buffer_size_ = other.m_buffer_size_;
        m_allocation_ = other.m_allocation_;
        m_instance_size_ = other.m_instance_size_;
        m_alignment_size_ = other.m_alignment_size_;
        m_instance_size_ = other.m_instance_size_;
        other.m_device_wrapper_ = nullptr;
    }
    return *this;
}

DeviceBufferWrapper::~DeviceBufferWrapper() {
    vkDeviceWaitIdle(*m_device_wrapper_);
    vmaDestroyBuffer(m_device_wrapper_->get_allocator(), m_buffer_, m_allocation_);
}

void DeviceBufferWrapper::write_to_buffer(void* data, VkDeviceSize size,
    VkDeviceSize offset) const {
    if (size == VK_WHOLE_SIZE) {
        vmaCopyMemoryToAllocation(m_device_wrapper_->get_allocator(), data, m_allocation_, offset, m_buffer_size_);
    } else {
        char* mapped_data = static_cast<char*>(data);
        mapped_data += offset;
        vmaCopyMemoryToAllocation(m_device_wrapper_->get_allocator(), mapped_data, m_allocation_, offset, size);
    }
}

void DeviceBufferWrapper::write_to_index(void* data, int index) const {
    write_to_buffer(data, m_instance_size_, index * m_alignment_size_);
}

VkResult DeviceBufferWrapper::flush(VkDeviceSize size,
    VkDeviceSize offset) const {
    return vmaFlushAllocation(m_device_wrapper_->get_allocator(), m_allocation_, offset, size);
}

VkResult DeviceBufferWrapper::flush_index(int index) const {
    return flush(m_alignment_size_, index * m_alignment_size_);
}

VkResult DeviceBufferWrapper::invalidate(VkDeviceSize size, VkDeviceSize offset) const {
    return vmaInvalidateAllocation(m_device_wrapper_->get_allocator(), m_allocation_, offset, size);
}

VkResult DeviceBufferWrapper::invalidate_index(int index) const {
    return invalidate(m_alignment_size_, index * m_alignment_size_);
}

VkBuffer DeviceBufferWrapper::buffer() const {
    return m_buffer_;
}

}