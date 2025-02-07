#include "String.h"

#include <cctype>
#include <cstring>

#include "Memory/Arena.h"

String::String(Arena& arena, size_t length) : m_length_(length), m_str_(static_cast<byte*>(arena.push(m_length_))) {
    
}

String::String(Arena& arena, const char* c_str) : m_length_(strlen(c_str)), m_str_(static_cast<byte*>(arena.push(m_length_ - 1))) {
    // Copy all but null byte. Won't need it.
    memcpy(m_str_, c_str, m_length_ - 1);
}

String::String(String&& other) noexcept : m_length_(other.m_length_), m_str_(other.m_str_) {
    
}

String& String::operator=(String&& other) noexcept {
    if (this != &other) {
        m_length_ = other.m_length_;
        m_str_ = other.m_str_;
    }
    return *this;
}

size_t String::length() const {
    return m_length_;
}

const char* String::c_str(Arena& arena) const {
    const size_t new_length = m_length_ + 1;
    char* str = static_cast<char*>(arena.push(new_length));
    memcpy(str, m_str_, m_length_);
    str[m_length_] = '\0';
    return str;
}

String String::substr(Arena& new_str_arena, size_t start, size_t end) const {
    if (start >= end || end >= m_length_ || start >= m_length_) {
        // static_cast so compiler isn't confused about if 0 is a const char* or a length
        return String{new_str_arena, static_cast<size_t>(0)};
    }
    size_t len = end - start;
    String return_str{new_str_arena, len};
    for (size_t i = start; i < end; i++) {
        return_str[i] = m_str_[i];
    }
    return return_str;
}

String String::substr(Arena& new_str_arena, size_t start) const {
    return substr(new_str_arena, start, m_length_);
}

String String::upper(Arena& new_str_arena) const {
    String return_str{new_str_arena, m_length_};
    for (size_t i = 0; i < m_length_; i++) {
        return_str[i] = toupper(m_str_[i]);
    }
    return return_str;
}

String String::lower(Arena& new_str_arena) const {
    String return_str{new_str_arena, m_length_};
    for (size_t i = 0; i < m_length_; i++) {
        return_str[i] = tolower(m_str_[i]);
    }
    return return_str;
}

String String::reverse(Arena& new_str_arena) const {
    String return_str{new_str_arena, m_length_};
    size_t i = 0, j = m_length_ - 1;
    while (i < j) {
        return_str[i++] = m_str_[j--];
    }
    return return_str;
}

String String::concat(Arena& new_str_arena, const String& str) const {
    String new_str{new_str_arena, m_length_ + str.length()};
    memcpy(new_str.m_str_, m_str_, m_length_);
    memcpy(new_str.m_str_ + m_length_, str.m_str_, str.length());
    return new_str;
}

byte String::bound_check_access(size_t index, int* error_code) {
    if (index <= 0 || index >= m_length_) {
        *error_code = 1;
        return '\0';
    }
    *error_code = 0;
    return m_str_[index];
}

byte String::bound_check_access(size_t index, int* error_code) const {
    if (index <= 0 || index >= m_length_) {
        *error_code = 1;
        return '\0';
    }
    *error_code = 0;
    return m_str_[index];
}

byte& String::operator[](size_t index) {
    return m_str_[index];
}

byte& String::operator[](size_t index) const {
    return m_str_[index];
}