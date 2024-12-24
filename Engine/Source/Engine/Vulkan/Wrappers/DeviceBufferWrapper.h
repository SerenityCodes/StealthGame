#pragma once
#include "DeviceWrapper.h"

namespace engine::vulkan {

class DeviceBufferWrapper {
    DeviceWrapper* m_device_wrapper_;
    void* m_mapped_memory_ = nullptr;
    VkBuffer m_buffer_ = VK_NULL_HANDLE;
    VmaAllocation m_allocation_ = VK_NULL_HANDLE;

    VkDeviceSize m_alignment_size_;
    VkDeviceSize m_buffer_size_;
    VkDeviceSize m_instance_size_;

    static VkDeviceSize aligned_size(VkDeviceSize size, VkDeviceSize alignment);
public:
    DeviceBufferWrapper(DeviceWrapper* device_wrapper, VkDeviceSize instance_size, uint32_t instance_count, VkBufferUsageFlags flags, VmaAllocationCreateFlags allocation_flags, VkDeviceSize min_offset_alignment = 1);
    DeviceBufferWrapper(const DeviceBufferWrapper&) = delete;
    DeviceBufferWrapper& operator=(const DeviceBufferWrapper&) = delete;
    DeviceBufferWrapper(DeviceBufferWrapper&& other) noexcept;
    DeviceBufferWrapper& operator=(DeviceBufferWrapper&& other) noexcept;
    ~DeviceBufferWrapper();
    
    void write_to_buffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    void write_to_index(void* data, int index) const;
    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    VkResult flush_index(int index) const;
    VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
    VkResult invalidate_index(int index) const;
    
    VkBuffer buffer() const;
};

}
