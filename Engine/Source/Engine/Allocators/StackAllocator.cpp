#include "StackAllocator.h"

namespace allocators {

StackAllocator::ManagedPtr::ManagedPtr(void* ptr, StackAllocator& allocator,
                                       size_t bytes_allocated) :
    m_ptr_(ptr),
    m_bytes_allocated_(bytes_allocated),
    m_allocator_(&allocator) {}

StackAllocator::ManagedPtr::ManagedPtr(const ManagedPtr& other) {
    m_ptr_ = other.m_ptr_;
    m_bytes_allocated_ = other.m_bytes_allocated_;
    m_allocator_ = other.m_allocator_;
}

StackAllocator::ManagedPtr& StackAllocator::ManagedPtr::operator=(
    const ManagedPtr& other) {
    if (this != &other) {
        m_ptr_ = other.m_ptr_;
        m_bytes_allocated_ = other.m_bytes_allocated_;
        m_allocator_ = other.m_allocator_;   
    }
    return *this;
}

StackAllocator::ManagedPtr::ManagedPtr(ManagedPtr&& other) noexcept {
    m_ptr_ = other.m_ptr_;
    m_bytes_allocated_ = other.m_bytes_allocated_;
    m_allocator_ = other.m_allocator_;
    other.m_ptr_ = nullptr;
}

StackAllocator::ManagedPtr& StackAllocator::ManagedPtr::operator=(
    ManagedPtr&& other) noexcept {
    if (this != &other) {
        m_ptr_ = other.m_ptr_;
        m_bytes_allocated_ = other.m_bytes_allocated_;
        m_allocator_ = other.m_allocator_;
        other.m_ptr_ = nullptr;
    }
    return *this;
}

StackAllocator::ManagedPtr::~ManagedPtr() {
    if (m_ptr_ != nullptr) {
        m_allocator_->free_bytes(m_bytes_allocated_);
    }
}

StackAllocator::ManagedPtr::operator void*() const {
    return m_ptr_;
}

StackAllocator::StackAllocator(const uint32_t stack_size) {
    this->stack_size = stack_size;
    stack_start = new uint32_t[stack_size];
    stack_ptr = stack_start;
}

StackAllocator::~StackAllocator() {
    delete[] stack_start;
}

StackAllocator::ManagedPtr StackAllocator::allocate(size_t bytes_to_allocate) {
    void* ptr = allocate_raw(bytes_to_allocate);
    return {ptr, *this, bytes_to_allocate};
}

StackAllocator::ManagedPtr StackAllocator::allocate(size_t bytes_to_allocate,
    uint32_t amount) {
    return allocate(bytes_to_allocate * amount);
}

void* StackAllocator::allocate_raw(const size_t size) {
    if (stack_ptr + size > stack_start + stack_size) {
        return nullptr;
    }
    return allocate_unchecked(size);
}

void* StackAllocator::allocate_unchecked(const size_t size) {
    void* ptr = stack_ptr;
    stack_ptr += size;
    return ptr;
}

void StackAllocator::free_bytes(size_t bytes_to_free) {
    if (stack_ptr - bytes_to_free < stack_start) {
        return;
    }
    stack_ptr -= bytes_to_free;
}

void StackAllocator::free_to_marker(uint32_t* ptr) {
    if (ptr < stack_ptr || ptr >= stack_ptr + stack_size) {
        return;
    }
    free_unchecked(ptr);
}

void StackAllocator::free_unchecked(uint32_t* ptr) {
    stack_ptr = ptr;
}

void StackAllocator::clear() {
    stack_ptr = stack_start;
}

void StackAllocator::allocate_new_start() {
    delete[] stack_start;
    stack_start = new uint32_t[stack_size];
    stack_ptr = stack_start;
}

uint32_t* StackAllocator::get_current_pos() const {
    return stack_ptr;
}
}