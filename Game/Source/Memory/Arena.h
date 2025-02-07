#pragma once
#include "Allocators/StackAllocator.h"

class Arena {
    allocators::StackAllocator m_stack_;
public:
    Arena() = default;
    Arena(size_t size);
    ~Arena() = default;

    void* push(size_t size);
    void* push(size_t size, size_t alignment);
    void* push_zero(size_t size);
    void* push_zero(size_t size, size_t alignment);

    void pop(size_t size);

    size_t get_position() const;
    void set_position(uint64_t* position);
    void clear();
};
