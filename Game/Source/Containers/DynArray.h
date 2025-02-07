#pragma once

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include "Memory/Arena.h"

template <typename T>
class DynArray {
   Arena* m_alloc_; 
    size_t m_size_;
    size_t m_capacity_;
    T* m_data_ptr_;
public:
    class iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;
    private:
        size_t current_pos;
        const DynArray* array;
    public:
        iterator(const DynArray* array, const size_t pos) : current_pos(pos), array(array) {}

        reference operator*() const {
            return array->m_data_ptr_[current_pos];
        }

        iterator& operator++() {
            ++current_pos;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++current_pos;
            return tmp;
        }

        iterator& operator--() {
            --current_pos;
            return *this;
        }

        iterator operator--(int) {
            iterator tmp = *this;
            --current_pos;
            return tmp;
        }

        difference_type operator+(const iterator& other) const {
            return current_pos + other.current_pos;   
        }

        difference_type operator-(const iterator& other) const {
            return current_pos - other.current_pos;
        }

        bool operator==(const iterator& other) const {
            return current_pos == other.current_pos;
        }

        bool operator!=(const iterator& other) const {
            return !((*this) == other);
        }
    };

    class const_iterator {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = const T;
        using difference_type = std::ptrdiff_t;
        using pointer = const T*;
        using reference = const T&;
    private:
        size_t current_pos;
        const DynArray* array;
    public:
        explicit const_iterator(const DynArray* array, const size_t pos) : current_pos(pos), array(array) {}

        reference operator*() const {
            return array->m_data_ptr_[current_pos];
        }

        const_iterator& operator++() {
            ++current_pos;
            return *this;
        }

        const_iterator& operator--() {
            --current_pos;
            return *this;
        }

        difference_type operator+(const const_iterator& other) const {
            return current_pos + other.current_pos;
        }

        difference_type operator-(const const_iterator& other) const {
            return current_pos - other.current_pos;
        }

        bool operator==(const const_iterator& other) const {
            return current_pos == other.current_pos;
        }

        bool operator!=(const const_iterator& other) const {
            return !((*this) == other);
        }
    };

    explicit DynArray(Arena& arena);
    DynArray(Arena& arena, size_t size);
    DynArray(std::initializer_list<T> init, Arena& arena);
    template <typename It>
    DynArray(Arena& arena, It begin, It end);
    DynArray(const DynArray& other) noexcept;
    DynArray& operator=(const DynArray& other) noexcept;
    DynArray(DynArray&& other) noexcept;
    DynArray& operator=(DynArray&& other) noexcept;
    ~DynArray();

    [[nodiscard]] bool is_empty() const;
    void resize(size_t size);
    [[nodiscard]] size_t size() const;
    T* data() const;
    void clear();

    T& operator[](size_t index) const;
    T& operator[](size_t index);
    void push_back(const T& value);
    template <typename It>
    void add_from_container(It begin, It end);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;
};

template<typename T>
DynArray<T>::DynArray(Arena& arena) {
    m_alloc_ = &arena;
    m_size_ = 0;
    m_capacity_ = 0;
    m_data_ptr_ = nullptr;
}

template<typename T>
DynArray<T>::DynArray(Arena& arena, size_t size) {
    m_alloc_ = &arena;
    m_size_ = size;
    m_capacity_ = size;
    m_data_ptr_ = static_cast<T*>(arena.push(m_size_ * sizeof(T)));
}

template <typename T>
DynArray<T>::DynArray(std::initializer_list<T> init, Arena& arena) {
    m_alloc_ = &arena;
    m_size_ = init.size();
    m_capacity_ = init.size();
    m_data_ptr_ = static_cast<T*>(arena.push(m_size_ * sizeof(T)));
    std::copy(init.begin(), init.end(), m_data_ptr_);
}

template <typename T>
template <typename It>
DynArray<T>::DynArray(Arena& arena, It begin, It end) {
    m_alloc_ = &arena;
    m_size_ = std::distance(begin, end);
    m_capacity_ = m_size_;
    m_data_ptr_ = static_cast<T*>(arena.push(m_size_ * sizeof(T)));
    std::copy(begin, end, m_data_ptr_);
}

template<typename T>
DynArray<T>::DynArray(const DynArray &other) noexcept {
    m_alloc_ = &other.m_alloc_;
    m_size_ = other.m_size_;
    m_capacity_ = other.m_capacity_;
    m_data_ptr_ = static_cast<T*>(other.m_alloc_->push(sizeof(T) * other.m_size_));
    std::copy(other.begin(), other.end(), m_data_ptr_);
}

template<typename T>
DynArray<T>& DynArray<T>::operator=(const DynArray& other) noexcept {
    if (this != &other) {
        m_alloc_ = &other.m_alloc_;
        m_size_ = other.m_size_;
        m_capacity_ = other.m_capacity_;
        m_data_ptr_ = static_cast<T*>(m_alloc_->push(m_size_ * sizeof(T)));
        std::copy(other.begin(), other.end(), m_data_ptr_);
        return *this;
    }
    return *this;
}

template<typename T>
DynArray<T>::DynArray(DynArray&& other) noexcept {
    m_alloc_ = &other.m_alloc_;
    m_size_ = other.m_size_;
    m_capacity_ = other.m_capacity_;
    this->m_data_ptr_ = other.m_data_ptr_;
    other.m_data_ptr_ = nullptr;
}

template <typename T>
DynArray<T>& DynArray<T>::operator=(DynArray&& other) noexcept {
    if (this != &other) {
        this->m_size_ = other.m_size_;
        this->m_data_ptr_ = other.m_data_ptr_;
        other.m_data_ptr_ = nullptr;
    }
    return *this;
}

template<typename T>
DynArray<T>::~DynArray() {
    
}

template<typename T>
bool DynArray<T>::is_empty() const {
    return m_size_ == 0;
}

template<typename T>
void DynArray<T>::resize(const size_t new_capacity) {
    T* new_data = static_cast<T*>(m_alloc_->push(new_capacity * sizeof(T)));
    std::copy(begin(), end(), new_data);
    m_data_ptr_ = new_data;
    this->m_capacity_ = new_capacity;
    // Pop old data off. However, using stack allocator here
}

template<typename T>
size_t DynArray<T>::size() const {
    return m_size_;
}

template<typename T>
T* DynArray<T>::data() const {
    return m_data_ptr_;
}

template <typename T>
void DynArray<T>::clear() {
    m_data_ptr_ = nullptr;
    m_size_ = 0;
}

template<typename T>
T& DynArray<T>::operator[](size_t index) const {
    return m_data_ptr_[index];
}

template<typename T>
T& DynArray<T>::operator[](size_t index) {
    return m_data_ptr_[index];
}

template <typename T>
void DynArray<T>::push_back(const T& value) {
    if (m_capacity_ == 0) {
        constexpr int default_capacity = 32;
        m_data_ptr_ = static_cast<T*>(m_alloc_->push(sizeof(T) * default_capacity));
        m_capacity_ = default_capacity;
        m_size_ = 0;
    } else if (m_size_ + 1 > m_capacity_) {
        resize(m_capacity_ * 2);
    }
    m_data_ptr_[m_size_++] = value;
}

template <typename T>
template <typename It>
void DynArray<T>::add_from_container(It begin, It end) {
    for (auto it = begin; it != end; ++it) {
        push_back(*it);
    }
}

template<typename T>
typename DynArray<T>::iterator DynArray<T>::begin() {
    return DynArray::iterator(this, 0);
}

template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::begin() const {
    return DynArray::const_iterator(this, 0);
}

template<typename T>
typename DynArray<T>::iterator DynArray<T>::end() {
    return DynArray::iterator(this, m_size_);
}

template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::end() const {
    return DynArray::const_iterator(this, m_size_);
}

template<typename T>
typename DynArray<T>::const_iterator DynArray<T>::cbegin() const {
    return DynArray::const_iterator(this, 0);
}

template<typename T>
typename DynArray<T>::const_iterator DynArray<T>::cend() const {
    return DynArray::const_iterator(this, m_size_);
}
