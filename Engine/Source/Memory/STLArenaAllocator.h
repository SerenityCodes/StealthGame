#pragma once
#include "Arena.h"

template <typename T>
class STLArenaAllocator {
    Arena* m_arena_;
public:
    using value_type = T;

    template <typename U>
    struct rebind {
        using other = STLArenaAllocator<U>;
    };

    STLArenaAllocator(Arena* allocator) noexcept : m_arena_(allocator) {}
    template <typename U>
    STLArenaAllocator(const STLArenaAllocator<U>& other) noexcept : m_arena_(other.get_arena()) {}

    value_type* allocate(size_t n) noexcept {
        return static_cast<T*>(m_arena_->push(sizeof(value_type) * n));
    }

    void deallocate(value_type* p, size_t n) noexcept {
        // Free is not supported in an arena        
    }

    Arena* get_arena() const noexcept {
        return m_arena_;
    }

    template <typename U>
    bool operator==(const STLArenaAllocator<U>& other) const noexcept {
        return m_arena_ == other.m_arena_;
    }

    template <typename U>
    bool operator!=(const STLArenaAllocator& other) const noexcept {
        return m_arena_ != other.m_arena_;
    }
};
