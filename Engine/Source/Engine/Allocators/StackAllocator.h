#pragma once
#include <utility>
#include <vector>

namespace allocators
{
    class StackMemoryPool {
        size_t m_size_;
        uint8_t* m_data_;
        size_t m_current_offset_;
    public:
        explicit StackMemoryPool(size_t size) {
            m_size_ = size;
            m_data_ = new uint8_t[m_size_];
            m_current_offset_ = 0;
        }
        
        ~StackMemoryPool() {
            delete[] m_data_;
        }

        void* allocate(size_t bytes_to_allocate) noexcept {
            size_t offset = m_current_offset_ + bytes_to_allocate;
            void* ptr = m_data_ + offset;
            m_current_offset_ += bytes_to_allocate;
            return ptr;
        }
        void deallocate(size_t bytes_to_deallocate) {
            m_current_offset_ -= bytes_to_deallocate;
        }

        void clear() {
            m_current_offset_ = 0;
        }

        bool operator==(const StackMemoryPool& other) const {
            return m_data_ == other.m_data_;
        }

        bool operator!=(const StackMemoryPool& other) const {
            return !(*this == other);
        }
    };

    template <typename T>
    class StackAllocator {
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T*;
        using size_type = std::size_t;
        using void_pointer = void*;
        using const_void_pointer = const void*;
        using difference_type = ptrdiff_t;

        using raw_pointer = uint8_t*;

        template <typename U>
        struct rebind {
            using other = StackAllocator<U>;  
        };
        
        using ToVoidAllocator = typename rebind<void>::other;

        template <typename U>
        friend class StackAllocator;
    private:
        StackMemoryPool* m_memory_pool_;
        bool m_own_memory_pool_;

    public:
        StackAllocator();
        explicit StackAllocator(size_t size);
        explicit StackAllocator(StackMemoryPool* memory_pool);
        ~StackAllocator();

        explicit StackAllocator(const StackAllocator& other);
        template <typename U>
        StackAllocator(const StackAllocator<U>& other,
            typename std::enable_if<!std::is_same<U, T>::value, void*>::type = nullptr);
        explicit StackAllocator(StackAllocator&& other) noexcept;
        template <typename U>
        StackAllocator(StackAllocator<U>&& other,
            typename std::enable_if<!std::is_same<U, T>::value, void*>::type = nullptr) noexcept;

        pointer allocate(size_type number_to_allocate);
        void deallocate(pointer p, size_type number_to_deallocate);
        void free(size_type bytes) const;
        template <typename... Args>
        void construct(pointer p, Args&&... args);
        void destroy(pointer p);
        void clear() const;

        bool operator==(const StackAllocator& other) const;
        bool operator!=(const StackAllocator& other) const;
    };

    template <typename T>
    StackAllocator<T>::StackAllocator() : m_memory_pool_(new StackMemoryPool(2 << 20)), m_own_memory_pool_(true) {
        
    }

    template <typename T>
    StackAllocator<T>::StackAllocator(size_t size) : m_memory_pool_(new StackMemoryPool(size)), m_own_memory_pool_(true) {
        
    }

    template <typename T>
    StackAllocator<T>::StackAllocator(StackMemoryPool* memory_pool) : m_memory_pool_(memory_pool), m_own_memory_pool_(false) { }

    template <typename T>
    StackAllocator<T>::~StackAllocator() {
        if (m_own_memory_pool_) {
            delete m_memory_pool_;
        }
    }

    template <typename T>
    StackAllocator<T>::StackAllocator(const StackAllocator& other) {
        m_memory_pool_ = other.m_memory_pool_;
        m_own_memory_pool_ = false;
    }

    template <typename T>
    template <typename U>
    StackAllocator<T>::StackAllocator(const StackAllocator<U>& other,
        typename std::enable_if<!std::is_same<U, T>::value, void*>::type) : m_memory_pool_(other.m_memory_pool_), m_own_memory_pool_(false) {
        
    }

    template <typename T>
    StackAllocator<T>::StackAllocator(StackAllocator&& other) noexcept {
        m_memory_pool_ = other.m_memory_pool_;
        m_own_memory_pool_ = other.m_own_memory_pool_;
        other.m_memory_pool_ = nullptr;
        other.m_own_memory_pool_ = false;
    }

    template <typename T>
    template <typename U>
    StackAllocator<T>::StackAllocator(StackAllocator<U>&& other,
        typename std::enable_if<!std::is_same<U, T>::value, void*>::type) noexcept {
        auto other_ = std::move(other);
        m_memory_pool_ = other_.m_memory_pool_;
        m_own_memory_pool_ = other_.m_own_memory_pool_;
        other_.m_memory_pool_ = nullptr;
        other_.m_own_memory_pool_ = false;
    }

    template <typename T>
    typename StackAllocator<T>::pointer StackAllocator<T>::allocate(size_type number_to_allocate) {
        return static_cast<T*>(m_memory_pool_->allocate(sizeof(T) * number_to_allocate));
    }

    template <typename T>
    void StackAllocator<T>::deallocate(pointer p, size_type number_to_deallocate) {
        // Won't deallocate memory
    }

    template <typename T>
    void StackAllocator<T>::free(size_type bytes) const {
        if (m_memory_pool_) {
            m_memory_pool_->deallocate(bytes);
        }
    }

    template <typename T>
    template <typename ... Args>
    void StackAllocator<T>::construct(pointer p, Args&&... args) {
        new(p) T(std::forward<Args>(args)...);
    }

    template <typename T>
    void StackAllocator<T>::destroy(pointer p) {
        p->~T();
    }

    template <typename T>
    void StackAllocator<T>::clear() const {
        if (m_memory_pool_ && m_own_memory_pool_) {
            m_memory_pool_->clear();
        }
    }

    template <typename T>
    bool StackAllocator<T>::operator==(const StackAllocator& other) const {
        return m_memory_pool_ == other.m_memory_pool_;
    }

    template <typename T>
    bool StackAllocator<T>::operator!=(const StackAllocator& other) const {
        return m_memory_pool_ != other.m_memory_pool_;
    }

    template <typename T>
    using stack_vec = std::vector<T, allocators::StackAllocator<T>>;

}

