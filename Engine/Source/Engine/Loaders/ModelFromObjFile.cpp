#include "ModelFromObjFile.h"

#include <fstream>

engine::Model engine::load_model_from_obj_file(Arena& temp_arena, Arena& model_arena, const char* file_name) {
    std::ifstream file(file_name);
    Model model;
    model.vertices = DynArray<vulkan::VulkanModel::Vertex>{model_arena};
    model.indices = DynArray<uint32_t>{model_arena};
    if (!file.is_open()) {
        return model;
    }
    
}