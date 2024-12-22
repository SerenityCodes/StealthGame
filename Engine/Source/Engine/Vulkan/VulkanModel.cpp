#include "VulkanModel.h"

#include <array>
#include <fstream>

#include <unordered_map>

#include "Memory/STLArenaAllocator.h"
#include "assimp/Importer.hpp"
#include "assimp/StringComparison.h"
#include "assimp/Vertex.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include <filesystem>

namespace engine::vulkan {

VkVertexInputBindingDescription VulkanModel::Vertex::
get_binding_descriptions() {
    VkVertexInputBindingDescription binding_description;
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 4> VulkanModel::Vertex::
get_attribute_descriptions() {
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

void VulkanModel::VertexIndexInfo::load_model(Arena& temp_arena, const char* file_path, uint32_t import_flags) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(file_path, import_flags);
    if (!scene) {
        std::cerr << "Failed to load model" << std::endl;
        return;
    }
    vertices.clear();
    indices.clear();
    
    process_node(scene->mRootNode, scene, vertices, indices);
}

void VulkanModel::create_buffer_from_staging(DeviceWrapper* device_wrapper, VkCommandPool command_pool, VkDeviceSize size, VkBufferUsageFlags usage, void* data_to_copy, VkBuffer& buffer, VkDeviceMemory& buffer_memory) {
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    device_wrapper->create_buffer(size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        &staging_buffer_memory);

    void* data;
    vkMapMemory(*device_wrapper, staging_buffer_memory, 0, size, 0, &data);
    memcpy(data, data_to_copy, size);
    vkUnmapMemory(*device_wrapper, staging_buffer_memory);
    
    device_wrapper->create_buffer(size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &buffer,
        &buffer_memory);

    device_wrapper->copy_buffer(command_pool, staging_buffer, buffer, size);

    vkDestroyBuffer(*device_wrapper, staging_buffer, nullptr);
    vkFreeMemory(*device_wrapper, staging_buffer_memory, nullptr);
}

VulkanModel::VulkanModel(DeviceWrapper* device_wrapper,
                         VkCommandPool command_pool, const VertexIndexInfo& vertices) : m_device_wrapper_(device_wrapper) {
    m_vertex_count_ = static_cast<uint32_t>(vertices.vertices.size());
    m_index_count_ = static_cast<uint32_t>(vertices.indices.size());
    assert(m_vertex_count_ > 3 && "Vertex count must be greater than 3");
    VkDeviceSize vertex_buffer_size = sizeof(Vertex) * m_vertex_count_;
    VkDeviceSize index_buffer_size = sizeof(uint32_t) * m_index_count_;
    create_buffer_from_staging(device_wrapper, command_pool, vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices.vertices.data(), m_vertex_buffer_, m_vertex_buffer_memory_);
    if (m_index_count_ > 0) {
        create_buffer_from_staging(device_wrapper, command_pool, index_buffer_size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, vertices.indices.data(), m_index_buffer_, m_index_buffer_memory_);
    }
}

VulkanModel::~VulkanModel() {
    vkDeviceWaitIdle(*m_device_wrapper_);
    vkDestroyBuffer(*m_device_wrapper_, m_vertex_buffer_, nullptr);
    vkFreeMemory(*m_device_wrapper_, m_vertex_buffer_memory_, nullptr);

    if (m_index_buffer_ != VK_NULL_HANDLE && m_index_count_ > 0) {
        vkDestroyBuffer(*m_device_wrapper_, m_index_buffer_, nullptr);
        vkFreeMemory(*m_device_wrapper_, m_index_buffer_memory_, nullptr);
    }
}

VulkanModel VulkanModel::load_model(Arena& temp_arena, Arena& model_arena, DeviceWrapper* device_wrapper, VkCommandPool command_pool, const char* file_path, uint32_t import_flags) {
    VertexIndexInfo vertex_index_info{model_arena};
    vertex_index_info.load_model(temp_arena, file_path, import_flags);
    return {device_wrapper, command_pool, vertex_index_info};
}

void VulkanModel::bind(VkCommandBuffer command_buffer) const {
    VkDeviceSize offset[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_vertex_buffer_, offset);
    if (m_index_count_ > 0) {
        vkCmdBindIndexBuffer(command_buffer, m_index_buffer_, 0, VK_INDEX_TYPE_UINT32);
    }
}

void VulkanModel::draw(VkCommandBuffer command_buffer) const {
    if (m_index_count_ > 0) {
        vkCmdDrawIndexed(command_buffer, m_index_count_, 1, 0, 0, 0);
    } else {
        vkCmdDraw(command_buffer, m_vertex_count_, 1, 0, 0);
    }
}

}