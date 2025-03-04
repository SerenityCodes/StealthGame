#pragma once
#include <fstream>

#include "common.h"
#include "Containers/DynArray.h"
#include "Containers/String.h"

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
    String m_file_path_;
public:
    RawFile(Arena* arena, const char* path);
    RawFile(const RawFile& other);
    RawFile& operator=(const RawFile& other);
    RawFile(RawFile&& other) noexcept;
    RawFile& operator=(RawFile&& other) noexcept;
    ~RawFile() = default;

    DynArray<byte> read_raw_bytes() const;
    [[nodiscard]] String get_file_extension() const;
    String& get_file_path();
    String copy_file_path() const;
};

class Folder {
    Arena* m_arena_;
    String m_folder_path_;
    DynArray<String> m_files_;
public:
    Folder(Arena* arena, const char* path);
    Folder(const Folder&) = delete;
    Folder& operator=(const Folder&) = delete;
    Folder(Folder&&) = delete;
    Folder& operator=(Folder&&) = delete;
    ~Folder() = default;

    DynArray<RawFile> read_all_files();
    RawFile read_file(const String& path) const;
    RawFile read_file(const char* path) const;
    RawFile read_file(u32 index);

    u64 get_amount_of_files() const;
};

}
