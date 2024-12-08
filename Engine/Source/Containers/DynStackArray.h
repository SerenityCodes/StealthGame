#pragma once
#include <cstdint>

namespace engine::containers {

template <typename T>
class DynStackArray {
    T* m_data_ptr_;
    size_t m_size_;
    size_t m_capacity_;
public:
    explicit DynStackArray(size_t size);
    DynStackArray(const DynStackArray& other) = delete;
    DynStackArray& operator=(const DynStackArray& other) = delete;
    DynStackArray(DynStackArray&& other) noexcept;
    DynStackArray& operator=(DynStackArray&& other) noexcept;
    ~DynStackArray();

    bool push(const T& element);
    T pop();
    bool empty() const;
};

template <typename T>
DynStackArray<T>::DynStackArray(size_t size) {
    m_data_ptr_ = new T[size];
    m_size_ = 0;
    m_capacity_ = size;
}

template <typename T>
DynStackArray<T>::DynStackArray(DynStackArray&& other) noexcept {
    m_data_ptr_ = other.m_data_ptr_;
    m_size_ = other.m_size_;
    m_capacity_ = other.m_capacity_;
}

template <typename T>
DynStackArray<T>& DynStackArray<T>::operator=(DynStackArray&& other) noexcept {
    if (this != &other) {
        delete[] m_data_ptr_;
        m_data_ptr_ = other.m_data_ptr_;
        m_size_ = other.m_size_;
        m_capacity_ = other.m_capacity_;
    }
    return *this;
}

template <typename T>
DynStackArray<T>::~DynStackArray() {
    delete[] m_data_ptr_;
}

template <typename T>
bool DynStackArray<T>::push(const T& element) {
    if (m_size_ >= m_capacity_) {
        return false;
    }
    m_data_ptr_[m_size_++] = element;
    return true;
}

template <typename T>
T DynStackArray<T>::pop() {
    if (m_size_ == 0) {
        return T();
    }
    return m_data_ptr_[m_size_--];
}

template <typename T>
bool DynStackArray<T>::empty() const {
    return m_size_ == 0;
}


}