#include "Arena.h"

#include <cstring>
#include <filesystem>

Arena::Arena(size_t size) : m_stack_(size) {
    
}

void* Arena::push(size_t size) {
    return m_stack_.allocate(size);
}

void* Arena::push(size_t size, size_t alignment) {
    return m_stack_.allocate(size, alignment);
}

void* Arena::push_zero(size_t size) {
    void* data = m_stack_.allocate(size);
    memset(data, 0, size);
    return data;
}

void* Arena::push_zero(size_t size, size_t alignment) {
    void* data = m_stack_.allocate(size, alignment);
    memset(data, 0, size);
    return data;
}

void Arena::pop(size_t size) {
    m_stack_.free_bytes(size);
}

size_t Arena::get_position() const {
    return m_stack_.get_stack_size();
}

void Arena::set_position(uint64_t* position) {
    m_stack_.free_to_marker(position);
}

void Arena::clear() {
    m_stack_.clear();
}