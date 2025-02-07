#pragma once
#include "common.h"
#include "Memory/Arena.h"

class String {
    size_t m_length_;
    byte* m_str_;
public:
    String(char* c_str);
    String(Arena& arena, size_t length);
    String(Arena& arena, const char* c_str);
    
    String(const String& other) = delete;
    String& operator=(const String& other) = delete;
    
    String(String&& other) noexcept;
    String& operator=(String&& other) noexcept;
    
    ~String() = default;
    
    size_t length() const;
    const char* c_str(Arena& arena) const;
    
    String substr(Arena& new_str_arena, size_t start, size_t end) const;
    String substr(Arena& new_str_arena, size_t start) const;
    String upper(Arena& new_str_arena) const;
    String lower(Arena& new_str_arena) const;
    String reverse(Arena& new_str_arena) const;
    String concat(Arena& new_str_arena, const String& str) const;
    
    byte bound_check_access(size_t index, int* error_code);
    byte bound_check_access(size_t index, int* error_code) const;
    byte& operator[](size_t index);
    byte& operator[](size_t index) const;
};
