#include "Memory/Allocators/LinearAllocator.h"

allocators::LinearAllocator::LinearAllocator(void* start, size_t size) : m_start_(static_cast<uint64_t*>(start)), m_size_(size) { }

void* allocators::LinearAllocator::allocate(size_t size) {
    void* result = m_start_ + m_size_;
    m_size_ += size;
    return result;
}