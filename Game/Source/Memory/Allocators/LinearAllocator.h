#pragma once
#include <cstdint>

namespace allocators {

class LinearAllocator {
    uint64_t* m_start_;
    size_t m_size_;
public:
    explicit LinearAllocator(void* start, size_t size);
    LinearAllocator(const LinearAllocator& other) = delete;
    LinearAllocator(LinearAllocator&& other) = delete;
    LinearAllocator& operator=(const LinearAllocator& other) = delete;
    LinearAllocator& operator=(LinearAllocator&& other) = delete;
    ~LinearAllocator() = default;

    void* allocate(size_t size);
};


}