#pragma once
#include <vulkan/vulkan_core.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "Containers/DynArray.h"

#include "common.h"
#include "Wrappers/DeviceBufferWrapper.h"

namespace engine::vulkan {

class VulkanModel {
public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;

        static VkVertexInputBindingDescription get_binding_descriptions();
        static std::array<VkVertexInputAttributeDescription, 4> get_attribute_descriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };

    struct VertexIndexInfo {
        VertexIndexInfo(Arena& model_arena);
        DynArray<Vertex> vertices;
        DynArray<uint32_t> indices;

        void load_model(Arena& temp_arena, const char* base_model_path, u32 import_flags = 0);
    };
    
private:
    VkDevice m_device_;
    size_t m_vertex_count_;
    size_t m_index_count_;
    DeviceBufferWrapper m_vertex_buffer_;
    DeviceBufferWrapper m_index_buffer_;

    static void create_buffer_from_staging(VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, void* data_to_copy, VkBuffer& buffer, VkDeviceMemory& buffer_memory);
public:
    VulkanModel(VulkanRenderer& renderer, const VertexIndexInfo& vertices);
    ~VulkanModel() = default;

    static VulkanModel load_model(Arena& temp_arena, Arena& model_arena, VulkanRenderer& renderer, const char* file_path);

    VulkanModel(const VulkanModel&) = delete;
    VulkanModel& operator=(const VulkanModel&) = delete;
    VulkanModel(VulkanModel&&) = delete;
    VulkanModel& operator=(VulkanModel&&) = delete;

    void bind(VkCommandBuffer command_buffer) const;
    void draw(VkCommandBuffer command_buffer) const;
};

}

template <typename T>
void hash_combine(std::size_t& seed, const T& value) {
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
template <>
struct hash<engine::vulkan::VulkanModel::Vertex> {
    size_t operator()(const engine::vulkan::VulkanModel::Vertex& vertex) const noexcept {
        size_t seed = 0;
        hash_combine(seed, vertex.position.x);
        hash_combine(seed, vertex.position.y);
        hash_combine(seed, vertex.position.z);

        hash_combine(seed, vertex.color.x);
        hash_combine(seed, vertex.color.y);
        hash_combine(seed, vertex.color.z);

        hash_combine(seed, vertex.normal.x);
        hash_combine(seed, vertex.normal.y);
        hash_combine(seed, vertex.normal.z);

        hash_combine(seed, vertex.uv.x);
        hash_combine(seed, vertex.uv.y);
        return seed;
    }
};
}
