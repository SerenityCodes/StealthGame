#pragma once

#include "Engine/Vulkan/VulkanModel.h"


namespace engine {

struct Model {
    Model();
    DynArray<vulkan::VulkanModel::Vertex> vertices;
    DynArray<uint32_t> indices;
};

Model load_model_from_obj_file(Arena& temp_arena, Arena& model_arena, const char* file_name);

}