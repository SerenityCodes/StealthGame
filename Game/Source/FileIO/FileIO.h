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
    Arena& m_arena_;
    String m_file_path_;
    const char* m_file_path_c_;
public:
    RawFile(Resource resource, Arena& arena, const char* path);
    RawFile(const RawFile&) = delete;
    RawFile& operator=(const RawFile&) = delete;
    RawFile(RawFile&&) = delete;
    RawFile& operator=(RawFile&&) = delete;
    ~RawFile() = default;

    DynArray<byte> read_raw_bytes();
    [[nodiscard]] String get_file_extension() const;
    String get_file_path() const;
};

class Folder {
    Arena& m_arena_;
    DynArray<String> m_file_paths_;
public:
    Folder(Arena& arena, const char* path);
    Folder(const Folder&) = delete;
    Folder& operator=(const Folder&) = delete;
    Folder(Folder&&) = delete;
    Folder& operator=(Folder&&) = delete;
    ~Folder() = default;

    DynArray<RawFile> read_all_files();
    RawFile read_file(String path);
    RawFile read_file(const char* path);
    RawFile read_file(u32 index);

    u64 get_amount_of_files() const;
};

}
