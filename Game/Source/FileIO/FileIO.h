#pragma once
#include <fstream>

#include "common.h"

namespace io {

enum Resource: byte {
    TEXTURE,
    SHADER,
    OBJ,
    CPP_CODE,
    CPP_HEADER
};

class RawFile {
    Arena* m_arena_;
    arena_string m_file_path_;
public:
    RawFile(Arena* arena, const arena_string& file_path);
    RawFile(Arena* arena, const char* file_path);
    RawFile(const RawFile& other);
    RawFile& operator=(const RawFile& other);
    RawFile(RawFile&& other) noexcept;
    RawFile& operator=(RawFile&& other) noexcept;
    ~RawFile() = default;

    arena_vector<byte> read_raw_bytes() const;
    arena_string read_contents() const;
    [[nodiscard]] arena_string get_file_extension() const;
    arena_string& get_file_path();
    arena_string copy_file_path() const;
};

class Folder {
    Arena* m_arena_;
    arena_string m_folder_path_;
    arena_vector<arena_string> m_files_;
public:
    Folder(Arena* arena, const char* path);
    Folder(const Folder&) = delete;
    Folder& operator=(const Folder&) = delete;
    Folder(Folder&&) = delete;
    Folder& operator=(Folder&&) = delete;
    ~Folder() = default;

    arena_vector<RawFile> read_all_files();
    RawFile read_file(const arena_string& path) const;
    RawFile read_file(const char* path) const;
    RawFile read_file(u32 index) const;

    u64 get_amount_of_files() const;
};

}
