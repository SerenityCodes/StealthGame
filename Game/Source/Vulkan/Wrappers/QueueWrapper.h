#pragma once

#include <vulkan/vulkan.h>
#include <limits>

#include "Memory/Arena.h"

namespace engine::vulkan {

class QueueWrapper {
public:
    struct QueueFamily {
        uint32_t graphics_family_index{std::numeric_limits<uint32_t>::max()};
        uint32_t present_family_index{std::numeric_limits<uint32_t>::max()};

        [[nodiscard]] bool same_family() const {
            return graphics_family_index == present_family_index;
        }
    };
private:
    VkQueue m_queue_;
    uint32_t m_graphics_family_index_;
public:
    QueueWrapper() = default;
    QueueWrapper(VkDevice device, uint32_t graphics_family_index);
    ~QueueWrapper() = default;

    operator VkQueue() const;
    static QueueFamily find_indices(Arena& temp_arena, VkSurfaceKHR surface, VkPhysicalDevice physical_device);
    uint32_t graphics_family_index() const;
};

}
