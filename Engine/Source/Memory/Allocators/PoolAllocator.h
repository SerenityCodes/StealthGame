#pragma once
#include <cstdint>

#include "Containers/DynStackArray.h"

namespace engine::allocators {

class PoolAllocator {
    uint8_t* m_starting_pool_;
    size_t m_chunks_;
    size_t m_chunk_size_;
    containers::DynStackArray<size_t> m_free_list_;
public:
    PoolAllocator(size_t chunks, size_t chunk_size);
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;
    ~PoolAllocator();

    void* allocate();
    void deallocate(const void* ptr);

    [[nodiscard]] size_t available_chunks() const;
};

}
