#pragma once

#include <array>

#include "QueueWrapper.h"
#include "SurfaceWrapper.h"
namespace engine::vulkan {

class DeviceWrapper {
    VkDevice m_device_;
    VkPhysicalDevice m_physical_device_;
    QueueWrapper m_graphics_queue_;
    QueueWrapper m_present_queue_;
    QueueWrapper::QueueFamily m_graphics_queue_family_;
public:
    DeviceWrapper(Arena& temp_arena, VkSurfaceKHR surface, VkInstance instance, const std::array<const char*, 1>& validation_layers);
    ~DeviceWrapper();

    operator VkDevice() const;

    VkDevice get_logical_device() const;
    VkPhysicalDevice get_physical_device() const;
    QueueWrapper get_graphics_queue() const;
    QueueWrapper get_present_queue() const;
    QueueWrapper::QueueFamily get_graphics_queue_family() const;

    VkCommandBuffer get_one_time_command_buffer(VkCommandPool command_pool) const;
    void end_one_time_command_buffer(VkCommandPool command_pool, VkCommandBuffer cmd_buffer) const;
    void copy_buffer(VkCommandPool command_pool, VkBuffer source_buffer, VkBuffer dest_buffer, VkDeviceSize size) const;

    uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const;
    void create_buffer(VkDeviceSize size, VkBufferUsageFlags flags, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* buffer_memory) const;
};

}
