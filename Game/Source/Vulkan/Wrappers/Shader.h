#pragma once

#include <vulkan\vulkan.h>

#include "common.h"
#include "Vulkan/ShaderEnum.h"

namespace engine::vulkan {

struct Shader {
    ShaderEnum shader_type;
    VkShaderStageFlagBits shader_stage;
    std::vector<u32> spirv_code;
};

void load_shader(Shader& shader, const arena_string& full_path, const arena_string& extension);

}
