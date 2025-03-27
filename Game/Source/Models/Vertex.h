#pragma once

#include <vulkan/vulkan_core.h>
#include <glm/glm.hpp>

#include "common.h"

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
    arena_vector<Vertex> vertices;
    arena_vector<uint32_t> indices;

    void load_model(Arena& temp_arena, const arena_string& base_model_path, u32 import_flags = 0);
};

template <typename T>
void hash_combine(std::size_t& seed, const T& value) {
    seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(const Vertex& vertex) const noexcept {
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
