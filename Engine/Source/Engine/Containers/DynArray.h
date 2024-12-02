#pragma once

#ifndef DYNARRAY_H
#define DYNARRAY_H

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <iterator>

template <typename T>
class DynArray {
    size_t size;
    T* data_ptr;

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
            return array->data_ptr[current_pos];
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
            return array->data_ptr[current_pos];
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
    size = 0;
    data_ptr = nullptr;
}

template<typename T>
DynArray<T>::DynArray(size_t size) {
    this->size = size;
    data_ptr = new T[size];
}

template <typename T>
DynArray<T>::DynArray(std::initializer_list<T> init) {
    this->size = init.size();
    data_ptr = new T[size];
    std::copy(init.begin(), init.end(), data_ptr);
}

template <typename T>
template <typename It>
DynArray<T>::DynArray(It begin, It end) {
    this->size = std::distance(begin, end);
    data_ptr = new T[this->size];
    std::copy(begin, end, data_ptr);
}

template<typename T>
DynArray<T>::DynArray(const DynArray &other) noexcept {
    this->size = other.size;
    this->data_ptr = new T[size];
    std::copy(other.begin(), other.end(), data_ptr);
}

template<typename T>
DynArray<T>& DynArray<T>::operator=(const DynArray& other) noexcept {
    if (this != &other) {
        delete[] data_ptr;
        this->size = other.size;
        this->data_ptr = new T[size];
        memcpy(this->data_ptr, other.data_ptr, this->size * sizeof(T));
        return *this;
    }
    return *this;
}

template<typename T>
DynArray<T>::DynArray(DynArray&& other) noexcept {
    this->size = other.size;
    this->data_ptr = other.data_ptr;
    other.data_ptr = nullptr;
}

template <typename T>
DynArray<T>& DynArray<T>::operator=(DynArray&& other) noexcept {
    if (this != &other) {
        delete[] data_ptr;
        this->size = other.size;
        this->data_ptr = other.data_ptr;
        other.data_ptr = nullptr;
    }
    return *this;
}

template<typename T>
DynArray<T>::~DynArray() {
    delete[] data_ptr;
}

template<typename T>
bool DynArray<T>::is_empty() const {
    return size == 0;
}

template<typename T>
void DynArray<T>::resize(const size_t size) {
    T* new_data = new T[size];
    const T* old_data = data_ptr;
    std::copy(begin(), end(), new_data);
    data_ptr = new_data;
    this->size = size;
    delete[] old_data;
}

template<typename T>
size_t DynArray<T>::get_size() const {
    return size;
}

template<typename T>
T* DynArray<T>::data() const {
    return data_ptr;
}

template<typename T>
T& DynArray<T>::operator[](size_t index) const {
    return data_ptr[index];
}

template<typename T>
T& DynArray<T>::operator[](size_t index) {
    return data_ptr[index];
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
    return DynArray::iterator(this, size);
}

template <typename T>
typename DynArray<T>::const_iterator DynArray<T>::end() const {
    return DynArray::const_iterator(this, size);
}

template<typename T>
typename DynArray<T>::const_iterator DynArray<T>::cbegin() const {
    return DynArray::const_iterator(this, 0);
}

template<typename T>
typename DynArray<T>::const_iterator DynArray<T>::cend() const {
    return DynArray::const_iterator(this, size);
}


#endif

