#include "FileIO.h"

#include <filesystem>
#include <fstream>
#include <regex>

namespace io {

RawFile::RawFile(Arena* arena, const arena_string& file_path) : RawFile(arena, file_path.c_str()) {
    
}

RawFile::RawFile(Arena* arena, const char* path) : m_arena_(arena), m_file_path_(path, STLArenaAllocator<char>{arena}) {
    
}

RawFile::RawFile(const RawFile& other) : m_file_path_(other.copy_file_path()) {
    if (this != &other) {
        m_arena_ = other.m_arena_;
    }
}

RawFile& RawFile::operator=(const RawFile& other) {
    if (this != &other) {
        m_arena_ = other.m_arena_;
        m_file_path_ = other.copy_file_path();
    }
    return *this;
}

RawFile::RawFile(RawFile&& other) noexcept : m_arena_(other.m_arena_), m_file_path_(std::move(other.m_file_path_)) {
    
}

RawFile& RawFile::operator=(RawFile&& other) noexcept {
    if (this != &other) {
        m_arena_ = other.m_arena_;
        m_file_path_ = std::move(other.m_file_path_);
    }
    return *this;
}

arena_vector<byte> RawFile::read_raw_bytes() const {
    std::ifstream file{m_file_path_.data(), std::ios::binary | std::ios::ate};
    ENGINE_ASSERT(file.is_open(), "Failed to open file {}", m_file_path_.data())
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    arena_vector<byte> raw_bytes{static_cast<size_t>(file_size), STLArenaAllocator<byte>{m_arena_}};
    file.read(reinterpret_cast<char*>(raw_bytes.data()), file_size);
    file.close();
    return raw_bytes;
}

arena_string RawFile::read_contents() const {
    std::ifstream file{m_file_path_.data(), std::ios::binary | std::ios::ate};
    ENGINE_ASSERT(file.is_open(), "Failed to open file {}", m_file_path_.data())
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    arena_string raw_bytes{static_cast<u64>(file_size), ' ', STLArenaAllocator<char>{m_arena_}};
    file.read(raw_bytes.data(), file_size);
    file.close();
    return raw_bytes;
}

arena_string RawFile::get_file_extension() const {
    std::regex extension_regex{"\\.\\w+"};
    std::cmatch extension_match;
    
    if (std::regex_search(m_file_path_.c_str(), extension_match, extension_regex)) {
        return arena_string{extension_match.str().substr(1, extension_match.str().length()), STLArenaAllocator<char>{m_arena_}};
    }
    return {"", STLArenaAllocator<char>{m_arena_}};
}

arena_string& RawFile::get_file_path() {
    return m_file_path_;
}

arena_string RawFile::copy_file_path() const {
    return m_file_path_;
}

Folder::Folder(Arena* arena, const char* path) : m_arena_(arena), m_folder_path_(path, STLArenaAllocator<char>{arena}), m_files_(STLArenaAllocator<arena_string>{arena}) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            m_files_.emplace_back(entry.path().string(), STLArenaAllocator<arena_string>{arena});
        }
    }
}

arena_vector<RawFile> Folder::read_all_files() {
    arena_vector<RawFile> files = MAKE_ARENA_VECTOR(m_arena_, RawFile);
    files.reserve(m_files_.size());
    for (u32 i = 0; i < files.size(); i++) {
        files[i] = RawFile{m_arena_, m_files_[i]};
    }
    return files;
}

RawFile Folder::read_file(const arena_string& path) const {
    return RawFile{m_arena_, path};
}

RawFile Folder::read_file(const char* path) const {
    return read_file(arena_string{path, STLArenaAllocator<char>{m_arena_}});
}

RawFile Folder::read_file(u32 index) const {
   return read_file(m_files_[index]);
}

u64 Folder::get_amount_of_files() const {
    return m_files_.size();
}

}