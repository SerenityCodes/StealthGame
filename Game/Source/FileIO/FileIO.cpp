#include "FileIO.h"

#include <filesystem>
#include <fstream>
#include <regex>

#include "Containers/String.h"

namespace io {

RawFile::RawFile(Arena* arena, const char* path) : m_arena_(arena), m_file_path_(*arena, path) {
    
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

DynArray<byte> RawFile::read_raw_bytes() const {
    const char* c_path = m_file_path_.c_str(*m_arena_);
    std::ifstream file{c_path, std::ios::binary | std::ios::ate};
    ENGINE_ASSERT(file.is_open(), "Failed to open file {}", c_path)
    file.seekg(0, std::ios::end);
    const std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    DynArray<byte> raw_bytes{*m_arena_, static_cast<size_t>(file_size + 1)};
    file.read(reinterpret_cast<char*>(raw_bytes.data()), file_size);
    raw_bytes[file_size] = '\0';
    file.close();
    return raw_bytes;
}

String RawFile::get_file_extension() const {
    std::regex extension_regex{"\\.\\w+"};
    std::cmatch extension_match;
    
    if (std::regex_search(m_file_path_.c_str(*m_arena_), extension_match, extension_regex)) {
        return String{*m_arena_, extension_match.str().substr(1, extension_match.str().length()).c_str()};
    }
    return String{*m_arena_, ""};
}

String& RawFile::get_file_path() {
    return m_file_path_;
}

String RawFile::copy_file_path() const {
    return String{m_file_path_};
}

Folder::Folder(Arena* arena, const char* path) : m_arena_(arena), m_folder_path_(*arena, path), m_files_(*arena) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) {
            m_files_.emplace_back(*m_arena_, entry.path().string().c_str());
        }
    }
}

DynArray<RawFile> Folder::read_all_files() {
    DynArray<RawFile> files{*m_arena_, m_files_.size()};
    for (u32 i = 0; i < files.size(); i++) {
        files[i] = RawFile{m_arena_, m_files_[i].c_str(*m_arena_)};
    }
    return files;
}

RawFile Folder::read_file(const String& path) const {
    String full_path = path.prepend(*m_arena_, m_folder_path_);
    return RawFile{m_arena_, full_path.c_str(*m_arena_)};
}

RawFile Folder::read_file(const char* path) const {
    return read_file(String{*m_arena_, path});
}

RawFile Folder::read_file(u32 index) {
   return read_file(m_files_[index]);
}

u64 Folder::get_amount_of_files() const {
    return m_files_.size();
}

}