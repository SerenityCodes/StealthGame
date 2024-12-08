#include "MemoryArena/Allocators/LinearAllocator.h"

allocators::LinearAllocator::LinearAllocator(void* start, size_t size) : m_start_(static_cast<uint8_t*>(start)), m_size_(size) { }

void* allocators::LinearAllocator::allocate(size_t size) {
    m_size_ += size;
    void* result = m_start_ + m_size_;
    return result;
}