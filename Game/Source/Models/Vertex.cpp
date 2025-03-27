#include "Vertex.h"
#include <array>
#include <filesystem>
#include <fstream>

#include "assimp/Importer.hpp"
#include "assimp/mesh.h"
#include "assimp/scene.h"

VkVertexInputBindingDescription Vertex::get_binding_descriptions() {
    VkVertexInputBindingDescription binding_description{};
    binding_description.binding = 0;
    binding_description.stride = sizeof(Vertex);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding_description;
}

std::array<VkVertexInputAttributeDescription, 4> Vertex::get_attribute_descriptions() {
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

VertexIndexInfo::VertexIndexInfo(Arena& model_arena) : vertices(MAKE_ARENA_VECTOR(&model_arena, Vertex)), indices(MAKE_ARENA_VECTOR(&model_arena, u32)) {
    
}

void process_mesh(aiMesh* mesh, arena_vector<Vertex>& vertices, arena_vector<u32>& indices) {
    for (u32 i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

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

    for (u32 i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        indices.push_back(face.mIndices[0]);
        indices.push_back(face.mIndices[1]);
        indices.push_back(face.mIndices[2]);
    }
}

void process_node(aiNode* node, const aiScene* scene, arena_vector<Vertex>& vertices, arena_vector<u32>& indices) {
    for (u32 i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        process_mesh(mesh, vertices, indices);
    }
    for (u32 i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene, vertices, indices);
    }
}

void VertexIndexInfo::load_model(Arena& temp_arena, const arena_string& base_model_path, u32 import_flags) {
    arena_string obj_path = base_model_path + ".obj";
    arena_string model_path = base_model_path + ".processed";
    bool has_been_processed = std::filesystem::exists(model_path);
    if (std::filesystem::exists(obj_path) && !has_been_processed) {
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(obj_path.c_str(), import_flags);
        ENGINE_ASSERT(scene, "Failed to load model")
        vertices.clear();
        indices.clear();
    
        process_node(scene->mRootNode, scene, vertices, indices);
        vertices.resize(vertices.size());
        indices.resize(indices.size());
        
        // Write out custom format
        std::ofstream file{model_path.c_str()};
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
        std::ifstream file{model_path.c_str(), std::ios::binary};
        file.seekg(0, std::ios::beg);
        u32 vertices_count;
        file >> vertices_count;
        vertices.resize(vertices_count);
        for (u32 i = 0; i < vertices_count; i++) {
            Vertex vertex;
            file >> vertex.position.x >> vertex.position.y >> vertex.position.z;
            file >> vertex.color.r >> vertex.color.g >> vertex.color.b;
            file >> vertex.normal.x >> vertex.normal.y >> vertex.normal.z;
            file >> vertex.uv.x >> vertex.uv.y;
            vertices.push_back(vertex);
        }
        u32 indices_count;
        file >> indices_count;
        indices.resize(indices_count);
        for (u32 i = 0; i < indices_count; i++) {
            u32 index;
            file >> index;
            indices.push_back(index);
        }
    }
}
