#include "FileIO.h"

#include <fstream>
#include <regex>

#include "Containers/String.h"

namespace io {

RawFile::RawFile(Resource resource, Arena& arena, const char* path) : m_arena_(arena), m_file_path_(arena, path), m_file_path_c_(m_file_path_.c_str(arena)) {
    switch (resource) {
        case TEXTURE:
            m_file_path_ = m_file_path_.prepend(arena, String{arena, "Textures/"});
            break;
        case OBJ:
            m_file_path_ = m_file_path_.prepend(arena, String{arena, "Models/"});
            break;
        case SHADER:
            m_file_path_ = m_file_path_.prepend(arena, String{arena, "Shaders/"});
            break;
        case CPP_CODE:
        case CPP_HEADER:
            m_file_path_ = m_file_path_.prepend(arena, String{arena, "Source/"});
            break;
    }
}

DynArray<byte> RawFile::read_raw_bytes() {
    std::ifstream file{m_file_path_c_, std::ios::binary | std::ios::ate};
    ENGINE_ASSERT(file.is_open(), "Failed to open file {}", m_file_path_c_)
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    const size_t bytes_needed = sizeof(char) * file_size;
    DynArray<byte> raw_bytes{m_arena_, bytes_needed};
    file.read(reinterpret_cast<char*>(raw_bytes.data()), file_size);
    file.close();
    return raw_bytes;
}

String RawFile::get_file_extension() const {
    std::regex extension_regex{temp_string{"\\.\\w+", STLArenaAllocator<char>{&m_arena_}}};
    std::cmatch extension_match;
    
    if (std::regex_search(m_file_path_c_, extension_match, extension_regex)) {
        return String{m_arena_, extension_match.str().substr(1, extension_match.str().length()).c_str()};
    }
    return String{m_arena_, ""};
}

String RawFile::get_file_path() const {
    return String{m_arena_, m_file_path_.c_str(m_arena_)};
}

u64 Folder::get_amount_of_files() const {
    return m_file_paths_.size();
}

}