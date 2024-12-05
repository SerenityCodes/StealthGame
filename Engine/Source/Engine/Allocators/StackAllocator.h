#pragma once

#include <cstdint>

namespace allocators
{
    class StackAllocator {
    public:
        class ManagedPtr {
            void* m_ptr_;
            size_t m_bytes_allocated_;
            StackAllocator* m_allocator_;
        public:
            ManagedPtr(void* ptr, StackAllocator& allocator, size_t bytes_allocated);
            ManagedPtr(const ManagedPtr& other);
            ManagedPtr& operator=(const ManagedPtr& other);
            ManagedPtr(ManagedPtr&& other) noexcept;
            ManagedPtr& operator=(ManagedPtr&& other) noexcept;
            ~ManagedPtr();

            operator void*() const;
            

            template <typename Value_Type>
            Value_Type* get() const;
        };
    private:
        uint32_t* stack_start;
        uint32_t* stack_ptr;
        uint32_t stack_size;
    public:
        explicit StackAllocator(uint32_t stack_size);
        ~StackAllocator();
        
        ManagedPtr allocate(size_t bytes_to_allocate);
        ManagedPtr allocate(size_t bytes_to_allocate, uint32_t amount);
        void* allocate_raw(size_t size);
        void* allocate_unchecked(size_t size);
        void free_bytes(size_t bytes_to_free);
        void free_to_marker(uint32_t* ptr);
        void free_unchecked(uint32_t* ptr);
        void clear();
        void allocate_new_start();

        uint32_t* get_current_pos() const;
    };

    template <typename Value_Type>
    Value_Type* StackAllocator::ManagedPtr::get() const {
        return static_cast<Value_Type*>(m_ptr_);
    }
}

