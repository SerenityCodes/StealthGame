#include "Shader.h"

#include <regex>

#include "FileIO/FileIO.h"

namespace engine::vulkan {

void load_shader(Shader& shader, const arena_string& full_path, const arena_string& extension) {
    std::regex file_name_regex{"\\w+\\."};
    std::cmatch file_name_match;

    if (std::regex_search(full_path.data(), file_name_match, file_name_regex)) {
        std::string shader_name = file_name_match.str().substr(0, file_name_match.str().length() - 1);
        ENGINE_LOG_INFO("File name {}", shader_name)
        if (shader_name == "global") {
            if (extension == "frag") {
                shader.shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                shader.shader_type = GLOBAL_FRAGMENT_SHADER;
            }
            if (extension == "vert") {
                shader.shader_stage = VK_SHADER_STAGE_VERTEX_BIT;
                shader.shader_type = GLOBAL_VERTEX_SHADER;
            }
        }
    }
}

}