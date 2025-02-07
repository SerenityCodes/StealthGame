#pragma once

#include <cstdint>

namespace allocators
{
    class StackAllocator {
        uint64_t* m_data_;
        size_t m_size_;
        const size_t m_stack_size_;
    public:
        StackAllocator();
        explicit StackAllocator(size_t stack_size);
        StackAllocator(const StackAllocator& other) = delete;
        StackAllocator(StackAllocator&& other) = delete;
        StackAllocator& operator=(const StackAllocator& other) = delete;
        StackAllocator& operator=(StackAllocator&& other) = delete;
        ~StackAllocator();
        
        [[nodiscard]] void* allocate(size_t amount);
        void* allocate(size_t amount, size_t alignment);
        void free_bytes(size_t bytes_to_free);
        void free_to_marker(uint64_t* ptr);
        void clear();

        [[nodiscard]] uint64_t* get_current_pos() const;
        size_t get_stack_size() const;

        bool operator==(const StackAllocator&) const;
        bool operator!=(const StackAllocator&) const;
    };

}

