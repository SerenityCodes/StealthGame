#pragma once
#include "Memory/Arena.h"

namespace engine::allocators {

class PoolAllocator {
public:
    // Linked List structure
    struct Chunk {
        Chunk* next;
    };
private:
    Arena* m_allocation_arena_;
    Chunk* m_allocation_ptr_;
    size_t m_chunks_per_block_;
    size_t m_chunk_size_;
    
    Chunk* allocate_block() const;
public:
    PoolAllocator(Arena* allocation_arena, size_t chunks_per_block, size_t chunk_size);
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;
    ~PoolAllocator() = default;

    void* allocate();
    void deallocate(void* ptr);
};

}
