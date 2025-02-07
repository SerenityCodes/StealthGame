#pragma once
#include <array>

namespace containers {

template <typename T, size_t N>
class StackArray {
    std::array<T, N> m_array_;
    size_t m_size_;
public:
    StackArray();
    ~StackArray() = default;

    StackArray(const StackArray& other) = delete;
    StackArray& operator=(const StackArray& other) = delete;
    StackArray(StackArray&& other) = delete;
    StackArray& operator=(StackArray&& other) = delete;
    [[nodiscard]] size_t get_current_size() const;

    void push(T elem);
    T pop();

    void clear();
};

template <typename T, size_t N>
StackArray<T, N>::StackArray() : m_array_(), m_size_(0) { }

template <typename T, size_t N>
size_t StackArray<T, N>::get_current_size() const {
    return m_size_;
}

template <typename T, size_t N>
void StackArray<T, N>::push(T elem) {
    if (m_size_ >= N) {
        return;
    }
    m_array_[m_size_++] = elem;
}

template <typename T, size_t N>
T StackArray<T, N>::pop() {
    if (m_size_ == 0)
        return T();
    return m_array_[m_size_--];
}

template <typename T, size_t N>
void StackArray<T, N>::clear() {
    m_size_ = 0;
}

}




