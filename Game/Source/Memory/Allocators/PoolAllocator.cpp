#include "PoolAllocator.h"

#include <cstdlib>

namespace engine::allocators {

PoolAllocator::Chunk* PoolAllocator::allocate_block() const {
    size_t block_size = m_chunks_per_block_ * m_chunk_size_;
    Chunk* block_begin = static_cast<Chunk*>(m_allocation_arena_->push(block_size));

    Chunk* begin = block_begin;
    for (size_t i = 0; i < m_chunks_per_block_; i++) {
        begin->next = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(begin) + m_chunk_size_);
        begin = begin->next;
    }
    begin->next = nullptr;
    return block_begin;
}

PoolAllocator::PoolAllocator(Arena* allocation_arena, size_t chunks_per_block, size_t chunk_size) : m_allocation_arena_(allocation_arena), m_allocation_ptr_(nullptr), m_chunks_per_block_(chunks_per_block), m_chunk_size_(chunk_size) {}


void* PoolAllocator::allocate() {
    if (m_allocation_ptr_ == nullptr) {
        m_allocation_ptr_ = allocate_block();
    }
    Chunk* free_chunk = m_allocation_ptr_;
    m_allocation_ptr_ = m_allocation_ptr_->next;
    return free_chunk;
}

void PoolAllocator::deallocate(void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    Chunk* to_be_freed = static_cast<Chunk*>(ptr);
    to_be_freed->next = m_allocation_ptr_;
    m_allocation_ptr_ = to_be_freed;
}

}