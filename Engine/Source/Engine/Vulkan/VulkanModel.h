#pragma once
#include <vulkan/vulkan_core.h>

#include <glm/vec3.hpp>

#include "Engine/Containers/DynArray.h"
#include "Wrappers/DeviceWrapper.h"

namespace engine::vulkan {

class VulkanModel {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;

        static VkVertexInputBindingDescription get_binding_descriptions();
        static std::array<VkVertexInputAttributeDescription, 2> get_attribute_descriptions();
    };
private:
    DeviceWrapper* m_device_wrapper_;
    VkBuffer m_vertex_buffer_;
    VkDeviceMemory m_vertex_buffer_memory_;
    uint32_t m_vertex_count_;
public:
    VulkanModel(DeviceWrapper* device_wrapper, const std::vector<Vertex, allocators::StackAllocator<Vertex>>& vertices);
    ~VulkanModel();

    VulkanModel(const VulkanModel&) = delete;
    VulkanModel& operator=(const VulkanModel&) = delete;
    VulkanModel(VulkanModel&&) = delete;
    VulkanModel& operator=(VulkanModel&&) = delete;

    void bind(VkCommandBuffer command_buffer) const;
    void draw(VkCommandBuffer command_buffer) const;
};

}
