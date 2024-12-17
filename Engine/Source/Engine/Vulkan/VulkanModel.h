#pragma once
#include <vulkan/vulkan_core.h>

#include <glm/vec3.hpp>

#include "Containers/ArrayRef.h"
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

    struct VertexIndexInfo {
        ArrayRef<Vertex> vertices;
        ArrayRef<uint32_t> indices;
    };
private:
    DeviceWrapper* m_device_wrapper_;
    VkBuffer m_vertex_buffer_;
    VkBuffer m_index_buffer_;
    VkDeviceMemory m_vertex_buffer_memory_;
    VkDeviceMemory m_index_buffer_memory_;
    uint32_t m_vertex_count_;
    uint32_t m_index_count_;

    static void create_buffer_from_staging(DeviceWrapper* device_wrapper, VkCommandPool command_pool, VkDeviceSize size, VkBufferUsageFlags usage, void* data_to_copy, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
public:
    VulkanModel(DeviceWrapper* device_wrapper, VkCommandPool command_pool, const VertexIndexInfo& vertices);
    ~VulkanModel();

    VulkanModel(const VulkanModel&) = delete;
    VulkanModel& operator=(const VulkanModel&) = delete;
    VulkanModel(VulkanModel&&) = delete;
    VulkanModel& operator=(VulkanModel&&) = delete;

    void bind(VkCommandBuffer command_buffer) const;
    void draw(VkCommandBuffer command_buffer) const;
};

}
