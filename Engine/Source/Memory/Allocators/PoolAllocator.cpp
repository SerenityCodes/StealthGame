#include "PoolAllocator.h"

namespace engine::allocators {


PoolAllocator::PoolAllocator(size_t chunks, size_t chunk_size) : m_starting_pool_(new uint8_t[chunk_size * chunks]), m_chunks_(chunks), m_chunk_size_(chunk_size), m_free_list_(chunks) {
    for (size_t i = 0; i < chunks; i++) {
        m_free_list_.push(i);
    }
}

PoolAllocator::~PoolAllocator() {
    delete[] m_starting_pool_;
}

void* PoolAllocator::allocate() {
    if (m_free_list_.empty()) {
        return nullptr;
    }
    size_t free_index = m_free_list_.pop();
    m_chunks_--;
    return m_starting_pool_ + (free_index * m_chunk_size_);
}

void PoolAllocator::deallocate(const void* ptr) {
    if (ptr == nullptr) {
        return;
    }
    size_t free_index = (reinterpret_cast<size_t>(ptr) - reinterpret_cast<size_t>(m_starting_pool_)) / m_chunk_size_;
    m_free_list_.push(free_index);
    m_chunks_++;
}

size_t PoolAllocator::available_chunks() const {
    return m_chunks_;
}

}