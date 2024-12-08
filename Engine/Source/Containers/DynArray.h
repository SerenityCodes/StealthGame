#pragma once

#include <algorithm>
#include <functional>
#include <initializer_list>
#include <iterator>

template <typename T>
class DynArray {
    size_t m_size_;
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

    DynArray();
    explicit DynArray(size_t size);
    DynArray(std::initializer_list<T> init);
    template <typename It>
    DynArray(It begin, It end);
    DynArray(const DynArray& other) noexcept;
    DynArray& operator=(const DynArray& other) noexcept;
    DynArray(DynArray&& other) noexcept;
    DynArray& operator=(DynArray&& other) noexcept;
    ~DynArray();

    [[nodiscard]] bool is_empty() const;
    void resize(size_t size);
    [[nodiscard]] size_t get_size() const;
    T* data() const;
    void clear();

    T& operator[](size_t index) const;
    T& operator[](size_t index);

    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;

    const_iterator cbegin() const;
    const_iterator cend() const;
};

template<typename T>
DynArray<T>::DynArray() {
    m_size_ = 0;
    m_data_ptr_ = nullptr;
}

template<typename T>
DynArray<T>::DynArray(size_t size) {
    m_size_ = size;
    m_data_ptr_ = new T[m_size_];
}

template <typename T>
DynArray<T>::DynArray(std::initializer_list<T> init) {
    m_size_ = init.size();
    m_data_ptr_ = new T[init.size()];
    std::copy(init.begin(), init.end(), m_data_ptr_);
}

template <typename T>
template <typename It>
DynArray<T>::DynArray(It begin, It end) {
    m_size_ = std::distance(begin, end);
    m_data_ptr_ = new T[m_size_];
    std::copy(begin, end, m_data_ptr_);
}

template<typename T>
DynArray<T>::DynArray(const DynArray &other) noexcept {
    m_size_ = other.m_size_;
    m_data_ptr_ = new T[other.m_size_];
    std::copy(other.begin(), other.end(), m_data_ptr_);
}

template<typename T>
DynArray<T>& DynArray<T>::operator=(const DynArray& other) noexcept {
    if (this != &other) {
        delete[] m_data_ptr_;
        m_size_ = other.m_size_;
        m_data_ptr_ = new T[m_size_];
        std::copy(other.begin(), other.end(), m_data_ptr_);
        return *this;
    }
    return *this;
}

template<typename T>
DynArray<T>::DynArray(DynArray&& other) noexcept {
    m_size_ = other.m_size_;
    this->m_data_ptr_ = other.m_data_ptr_;
    other.m_data_ptr_ = nullptr;
}

template <typename T>
DynArray<T>& DynArray<T>::operator=(DynArray&& other) noexcept {
    if (this != &other) {
        delete[] m_data_ptr_;
        this->m_size_ = other.m_size_;
        this->m_data_ptr_ = other.m_data_ptr_;
        other.m_data_ptr_ = nullptr;
    }
    return *this;
}

template<typename T>
DynArray<T>::~DynArray() {
    delete[] m_data_ptr_;
}

template<typename T>
bool DynArray<T>::is_empty() const {
    return m_size_ == 0;
}

template<typename T>
void DynArray<T>::resize(const size_t size) {
    T* new_data = new T[size];
    const T* old_data = m_data_ptr_;
    std::copy(begin(), end(), new_data);
    m_data_ptr_ = new_data;
    this->m_size_ = size;
    delete[] old_data;
}

template<typename T>
size_t DynArray<T>::get_size() const {
    return m_size_;
}

template<typename T>
T* DynArray<T>::data() const {
    return m_data_ptr_;
}

template <typename T>
void DynArray<T>::clear() {
    delete[] m_data_ptr_;
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
