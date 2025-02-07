#include "StackAllocator.h"

#include <cstdlib>
#include <iostream>

#include "Logging/Logger.h"

namespace allocators {

static constexpr size_t DEFAULT_STACK_SIZE = 2 << 20;

StackAllocator::StackAllocator() : m_data_(new uint64_t[DEFAULT_STACK_SIZE]), m_size_(0), m_stack_size_(DEFAULT_STACK_SIZE) {
    
}

StackAllocator::StackAllocator(size_t stack_size) : m_data_(new uint64_t[stack_size]), m_size_(0), m_stack_size_(stack_size) {
    
}

StackAllocator::~StackAllocator() {
    delete[] m_data_;
}

void* StackAllocator::allocate(size_t amount) {
    if (amount == 0) {
        return nullptr;
    }
    uint64_t* current_pos = m_data_ + m_size_;
    if (m_size_ + amount > m_stack_size_) {
        // Introduce a linked list type setup with stack allocators
        return nullptr;
    }
    m_size_ += amount;
    return current_pos;
}

void* StackAllocator::allocate(size_t amount, size_t alignment) {
    if (amount == 0 || amount > m_stack_size_) {
        return nullptr;
    }
    uintptr_t current_pos = reinterpret_cast<uintptr_t>(m_data_) + m_size_;
    uintptr_t aligned_pos = (current_pos + (alignment - 1)) & ~(alignment - 1);
    size_t padding = aligned_pos - current_pos;
    size_t new_size = m_size_ + padding;
    if (new_size > m_stack_size_) {
        // Introduce a linked list type setup with stack allocators
        return nullptr;
    }
    m_size_ = new_size;
    return reinterpret_cast<void*>(aligned_pos);
}

void StackAllocator::free_bytes(size_t bytes_to_free) {
    if (bytes_to_free > m_size_) {
        ENGINE_LOG_ERROR("Cannot free {} bytes from stack allocator. Bigger than the actual size. Wiping stack allocator.", bytes_to_free)
        m_size_ = 0;
    } else {
        m_size_ -= bytes_to_free;
    }
}

void StackAllocator::free_to_marker(uint64_t* ptr) {
    size_t byte_difference = reinterpret_cast<uintptr_t>(ptr) - reinterpret_cast<uintptr_t>(m_data_);
    m_size_ = byte_difference;
}

void StackAllocator::clear() {
    m_size_ = 0;
}

uint64_t* StackAllocator::get_current_pos() const {
    return m_data_ + m_size_;
}

size_t StackAllocator::get_stack_size() const {
    return m_size_;
}

bool StackAllocator::operator==(const StackAllocator& other) const {
    return m_data_ == other.m_data_;
}

bool StackAllocator::operator!=(const StackAllocator& other) const {
    return !(*this == other);
}

}