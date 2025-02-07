#include "QueueWrapper.h"

namespace engine::vulkan {

QueueWrapper::QueueWrapper(VkDevice device, uint32_t graphics_family_index) : m_queue_(nullptr), m_graphics_family_index_(graphics_family_index) {
    vkGetDeviceQueue(device, graphics_family_index, 0, &m_queue_);
}

QueueWrapper::operator VkQueue() const {
    return m_queue_;
}

QueueWrapper::QueueFamily QueueWrapper::find_indices(Arena& temp_arena, const VkSurfaceKHR surface, const VkPhysicalDevice physical_device) {
    QueueFamily indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    VkQueueFamilyProperties* queue_families_arr = static_cast<VkQueueFamilyProperties*>(temp_arena.push_zero(queue_family_count * sizeof(VkQueueFamilyProperties)));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families_arr);
    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families_arr[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family_index = i;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if (present_support) {
            indices.present_family_index = i;
        }
        if (indices.graphics_family_index != std::numeric_limits<uint32_t>::max()) {
            break;
        }
    }
    return indices;
}

uint32_t QueueWrapper::graphics_family_index() const {
    return m_graphics_family_index_;
}

}