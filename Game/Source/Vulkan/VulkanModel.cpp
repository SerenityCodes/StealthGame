#include "VulkanModel.h"

#include <array>
#include <fstream>

#include "Memory/STLArenaAllocator.h"
#include <filesystem>
#include <assimp/Importer.hpp>

#include "common.h"
#include "assimp/mesh.h"
#include "assimp/scene.h"

namespace engine::vulkan {

VkVertexInputBindingDescription VulkanModel::Vertex::
get_binding_descriptions() {
    VkVertexInputBindingDescription binding_description;
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 4> VulkanModel::Vertex::get_attribute_descriptions() {
    VkVertexInputAttributeDescription position_attribute;
    position_attribute.binding = 0;
    position_attribute.location = 0;
    position_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    position_attribute.offset = offsetof(Vertex, position);

    VkVertexInputAttributeDescription color_attribute;
    color_attribute.binding = 0;
    color_attribute.location = 1;
    color_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    color_attribute.offset = offsetof(Vertex, color);

    VkVertexInputAttributeDescription normal_attribute;
    normal_attribute.binding = 0;
    normal_attribute.location = 2;
    normal_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    normal_attribute.offset = offsetof(Vertex, normal);

    VkVertexInputAttributeDescription texture_attribute;
    texture_attribute.binding = 0;
    texture_attribute.location = 3;
    texture_attribute.format = VK_FORMAT_R32G32B32_SFLOAT;
    texture_attribute.offset = offsetof(Vertex, uv);
    
    return {position_attribute, color_attribute, normal_attribute, texture_attribute};
}

VulkanModel::VertexIndexInfo::VertexIndexInfo(Arena& model_arena)
: vertices(model_arena), indices(model_arena) {
    
}

void process_mesh(aiMesh* mesh, DynArray<VulkanModel::Vertex>& vertices, DynArray<uint32_t>& indices) {
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        VulkanModel::Vertex vertex;

        vertex.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        if (mesh->HasNormals()) {
            vertex.normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
        } else {
            vertex.normal = {0.0f, 0.0f, 0.0f};
        }

        if (mesh->mTextureCoords[0]) {
            vertex.uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };    
        } else {
            vertex.uv = {0.f, 0.f};
        }

        if (mesh->HasVertexColors(0)) {
            vertex.color = {
            mesh->mColors[0][i].r,
            mesh->mColors[0][i].g,
            mesh->mColors[0][i].b
            };
        } else {
            vertex.color = {1.0f, 1.0f, 1.0f};
        }
        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
}

void process_node(aiNode* node, const aiScene* scene, DynArray<VulkanModel::Vertex>& vertices, DynArray<uint32_t>& indices) {
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, vertices, indices);
    }
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, vertices, indices);
    }
}

void VulkanModel::VertexIndexInfo::load_model(Arena& temp_arena, const char* base_model_path, u32 import_flags) {
    using temp_string = std::basic_string<char, std::char_traits<char>, STLArenaAllocator<char>>;
    temp_string obj_path = base_model_path + temp_string{".obj", STLArenaAllocator<char>{&temp_arena}};
    temp_string processed_path = base_model_path + temp_string{".processed", STLArenaAllocator<char>{&temp_arena}};
    bool has_been_processed = std::filesystem::exists(processed_path);
    if (std::filesystem::exists(obj_path) && !has_been_processed) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(obj_path.c_str(), import_flags);
        ENGINE_ASSERT(scene, "Failed to load model", 10, 20, 30)
        vertices.clear();
        indices.clear();
    
        process_node(scene->mRootNode, scene, vertices, indices);
        vertices.resize(vertices.size());
        indices.resize(indices.size());
        
        // Write out custom format
        std::ofstream file{processed_path.c_str()};
        file << vertices.size() << "\n";
        for (auto& [position, color, normal, uv] : vertices) {
            file << position.x << ' ' << position.y << ' ' << position.z << "\n";
            file << color.r << ' ' << color.g << ' ' << color.b << "\n";
            file << normal.x << ' ' << normal.y << ' ' << normal.z << "\n";
            file << uv.x << ' ' << uv.y << "\n";
        }
        file << indices.size() << "\n";
        for (auto& index : indices) {
            file << index << "\n";
        }
    } else if (has_been_processed) {
        std::ifstream file{processed_path.c_str(), std::ios::binary};
        file.seekg(0, std::ios::beg);
        uint32_t vertices_count;
        file >> vertices_count;
        vertices.resize(vertices_count);
        for (uint32_t i = 0; i < vertices_count; i++) {
            Vertex vertex;
            file >> vertex.position.x >> vertex.position.y >> vertex.position.z;
            file >> vertex.color.r >> vertex.color.g >> vertex.color.b;
            file >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;
            file >> vertex.uv.x >> vertex.uv.y;
            vertices.push_back(vertex);
        }
        uint32_t indices_count;
        file >> indices_count;
        indices.resize(indices_count);
        for (uint32_t i = 0; i < indices_count; i++) {
            uint32_t index;
            file >> index;
            indices.push_back(index);
        }
    }
}

VulkanModel::VulkanModel(VulkanRenderer& renderer, const VertexIndexInfo& vertices) : m_device_(renderer.vulkan_device()),
    m_vertex_buffer_(renderer, sizeof(Vertex), static_cast<uint32_t>(vertices.vertices.size()), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0),
    m_index_buffer_(renderer, sizeof(uint32_t), static_cast<uint32_t>(vertices.indices.size()), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, 0) {
    m_vertex_count_ = vertices.vertices.size();
    m_index_count_ = vertices.indices.size();
    assert(m_vertex_count_ > 3 && "Vertex count must be greater than 3");
    uint32_t tiny_vertex_count = static_cast<uint32_t>(vertices.vertices.size());
    uint32_t tiny_index_count = static_cast<uint32_t>(vertices.indices.size());
    
    DeviceBufferWrapper vertex_staging_buffer{renderer,
        sizeof(Vertex),
        tiny_vertex_count,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT};
    vertex_staging_buffer.write_to_buffer(vertices.vertices.data());
    renderer.copy_buffer(vertex_staging_buffer.buffer(), m_vertex_buffer_.buffer(), sizeof(Vertex) * tiny_vertex_count);

    DeviceBufferWrapper index_staging_buffer{renderer,
    sizeof(uint32_t),
    tiny_index_count,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT};
    index_staging_buffer.write_to_buffer(vertices.indices.data());
    renderer.copy_buffer(index_staging_buffer.buffer(), m_index_buffer_.buffer(), sizeof(uint32_t) * tiny_index_count);
}

VulkanModel VulkanModel::load_model(Arena& temp_arena, Arena& model_arena, VulkanRenderer& renderer, const char* file_path) {
    VertexIndexInfo vertex_index_info{model_arena};
    vertex_index_info.load_model(temp_arena, file_path);
    return vulkan::VulkanModel{renderer, vertex_index_info};
}

void VulkanModel::bind(VkCommandBuffer command_buffer) const {
    VkDeviceSize offset[] = {0};
    VkBuffer vertex_buffer = m_vertex_buffer_.buffer();
    VkBuffer index_buffer = m_index_buffer_.buffer();
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &vertex_buffer, offset);
    if (m_index_count_ > 0) {
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void VulkanModel::draw(VkCommandBuffer command_buffer) const {
    if (m_index_count_ > 0) {
        vkCmdDrawIndexed(command_buffer, static_cast<uint32_t>(m_index_count_), 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, static_cast<uint32_t>(m_vertex_count_), 1, 0, 0);
    }
}

}