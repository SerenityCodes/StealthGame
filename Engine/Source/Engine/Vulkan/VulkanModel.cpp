#include "VulkanModel.h"

#include <array>

namespace engine::vulkan {

VkVertexInputBindingDescription VulkanModel::Vertex::
get_binding_descriptions() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 2> VulkanModel::Vertex::
get_attribute_descriptions() {
    VkVertexInputAttributeDescription position_attribute{};
    position_attribute.binding = 0;
    position_attribute.location = 0;
    position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    position_attribute.offset = offsetof(Vertex, position);

    VkVertexInputAttributeDescription color_attribute{};
    color_attribute.binding = 0;
    color_attribute.location = 1;
    color_attribute.format = VK_FORMAT_R32G32_SFLOAT;
    color_attribute.offset = offsetof(Vertex, color);
    
    return {position_attribute, color_attribute};
}

VulkanModel::VulkanModel(DeviceWrapper* device_wrapper,
                         const ArrayRef<Vertex>& vertices) : m_device_wrapper_(device_wrapper) {
    m_vertex_count_ = static_cast<uint32_t>(vertices.size());
    assert(m_vertex_count_ > 3 && "Vertex count must be greater than 3");
    VkDeviceSize vertex_buffer_size = sizeof(Vertex) * m_vertex_count_;
    m_device_wrapper_->create_buffer(vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_vertex_buffer_,
        m_vertex_buffer_memory_);
    void* data;
    vkMapMemory(*m_device_wrapper_, m_vertex_buffer_memory_, 0, vertex_buffer_size, 0, &data);
    memcpy(data, vertices.data(), vertex_buffer_size);
    vkUnmapMemory(*m_device_wrapper_, m_vertex_buffer_memory_);
}

VulkanModel::~VulkanModel() {
    vkDeviceWaitIdle(*m_device_wrapper_);
    vkDestroyBuffer(*m_device_wrapper_, m_vertex_buffer_, nullptr);
    vkFreeMemory(*m_device_wrapper_, m_vertex_buffer_memory_, nullptr);
}

void VulkanModel::bind(VkCommandBuffer command_buffer) const {
    VkDeviceSize offset[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_vertex_buffer_, offset);
}

void VulkanModel::draw(VkCommandBuffer command_buffer) const {
    vkCmdDraw(command_buffer, m_vertex_count_, 1, 0, 0);
}

}