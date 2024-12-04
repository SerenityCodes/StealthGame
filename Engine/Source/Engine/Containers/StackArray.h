#pragma once
#include <cstdint>

namespace engine::containers {

template <typename T>
class StackArray {
    T* m_data_ptr_;
    size_t m_size_;
    size_t m_capacity_;
public:
    explicit StackArray(size_t size);
    StackArray(const StackArray& other) = delete;
    StackArray& operator=(const StackArray& other) = delete;
    StackArray(StackArray&& other) noexcept;
    StackArray& operator=(StackArray&& other) noexcept;
    ~StackArray();

    bool push(const T& element);
    T pop();
    bool empty() const;
};

template <typename T>
StackArray<T>::StackArray(size_t size) {
    m_data_ptr_ = new T[size];
    m_size_ = 0;
    m_capacity_ = size;
}

template <typename T>
StackArray<T>::StackArray(StackArray&& other) noexcept {
    m_data_ptr_ = other.m_data_ptr_;
    m_size_ = other.m_size_;
    m_capacity_ = other.m_capacity_;
}

template <typename T>
StackArray<T>& StackArray<T>::operator=(StackArray&& other) noexcept {
    if (this != &other) {
        delete[] m_data_ptr_;
        m_data_ptr_ = other.m_data_ptr_;
        m_size_ = other.m_size_;
        m_capacity_ = other.m_capacity_;
    }
    return *this;
}

template <typename T>
StackArray<T>::~StackArray() {
    delete[] m_data_ptr_;
}

template <typename T>
bool StackArray<T>::push(const T& element) {
    if (m_size_ >= m_capacity_) {
        return false;
    }
    m_data_ptr_[m_size_++] = element;
    return true;
}

template <typename T>
T StackArray<T>::pop() {
    if (m_size_ == 0) {
        return T();
    }
    return m_data_ptr_[m_size_--];
}

template <typename T>
bool StackArray<T>::empty() const {
    return m_size_ == 0;
}


}