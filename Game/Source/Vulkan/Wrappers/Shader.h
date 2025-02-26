#pragma once

#include <vulkan\vulkan.h>

#include "common.h"
#include "Containers/DynArray.h"
#include "FileIO/FileIO.h"
#include "Vulkan/ShaderEnum.h"

namespace engine::vulkan {

struct Shader {
    ShaderEnum shader_type;
    VkShaderStageFlagBits shader_stage;
    std::vector<u32> spirv_code;
};

Shader load_shader(Arena& temp_arena, const String& full_path, const String& extension);

}
