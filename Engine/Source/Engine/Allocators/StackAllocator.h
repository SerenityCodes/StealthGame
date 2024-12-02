#pragma once

#include <cstdint>

namespace allocators
{
    class StackAllocator {
        uint32_t* stack_start;
        uint32_t* stack_ptr;
        uint32_t stack_size;
    public:
        explicit StackAllocator(uint32_t stack_size);
        ~StackAllocator();

        void* allocate(uint32_t size);
        void* allocate_unchecked(uint32_t size);
        void free_to_marker(uint32_t* ptr);
        void free_unchecked(uint32_t* ptr);
        void clear();
        void allocate_new_start();
    };
}

