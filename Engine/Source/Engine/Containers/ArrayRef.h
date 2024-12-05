#pragma once
#include <iterator>

template <typename T>
class ArrayRef {
    T* m_data_;
    size_t m_size_;
    bool m_owned_;
public:
    ArrayRef() = default;
    ArrayRef(const std::initializer_list<T>& init);
    ArrayRef(T* data, size_t size);
    ArrayRef(T* data, size_t size, bool owned);
    ArrayRef(const ArrayRef& other) = delete;
    ArrayRef(ArrayRef&& other) noexcept;
    ArrayRef& operator=(const ArrayRef& other) = delete;
    ArrayRef& operator=(ArrayRef&& other) noexcept;
    ~ArrayRef();
    
    T& operator[](size_t index);
    T& operator[](size_t index) const;
    [[nodiscard]] size_t size() const;
    T* data() const;
    bool is_empty() const;

    template <typename Ptr, typename Ref>
    class iterator {
        Ptr ptr;
        size_t index;
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = Ptr;
        using reference = Ref;
        
        iterator(Ptr ptr, size_t index) : ptr(ptr), index(index) {}

        iterator& operator++() {
            index++;
            return *this;
        }

        iterator& operator--() {
            index--;
            return *this;
        }

        reference operator*() {
            return *(ptr + index);
        }

        difference_type operator-(const iterator& other) {
            return index - other.index;
        }

        difference_type operator+(const iterator& other) {
            return index + other.index;   
        }

        bool operator==(const iterator& other) {
            return index == other.index;
        }

        bool operator!=(const iterator& other) {
            return index != other.index;
        }
    };

    using non_const_iterator = iterator<T*, T&>;
    using const_iterator = iterator<const T*, const T&>;

    non_const_iterator begin() {
        return non_const_iterator(m_data_, 0);
    }

    non_const_iterator end() {
        return non_const_iterator(m_data_, m_size_);
    }

    const_iterator begin() const {
        return const_iterator(m_data_, 0);
    }

    const_iterator end() const {
        return const_iterator(m_data_, m_size_);
    }

    const_iterator cbegin() const {
        return begin();
    }

    const_iterator cend() const {
        return end();
    }
};

template <typename T>
ArrayRef<T>::ArrayRef(const std::initializer_list<T>& init) {
    m_data_ = new T[init.size()];
    std::copy(init.begin(), init.end(), m_data_);
    m_size_ = init.size();
    m_owned_ = true;
}

template <typename T>
ArrayRef<T>::ArrayRef(T* data, size_t size) {
    this->m_data_ = data;
    this->m_size_ = size;
    this->m_owned_ = false;
}

template <typename T>
ArrayRef<T>::ArrayRef(T* data, size_t size, bool owned) {
    this->m_data_ = data;
    this->m_size_ = size;
    this->m_owned_ = owned;
}

template <typename T>
ArrayRef<T>::ArrayRef(ArrayRef&& other) noexcept {
    this->m_data_ = other.m_data_;
    this->m_size_ = other.m_size_;
    this->m_owned_ = other.m_owned_;
    other.m_data_ = nullptr;
}

template <typename T>
ArrayRef<T>& ArrayRef<T>::operator=(ArrayRef&& other) noexcept {
    if (this != &other) {
        this->m_data_ = other.m_data_;
        this->m_size_ = other.m_size_;
        this->m_owned_ = other.m_owned_;
        other.m_data_ = nullptr;
    }
    return *this;
}

template <typename T>
ArrayRef<T>::~ArrayRef() {
    if (m_owned_) {
        delete[] m_data_;
    }
}

template <typename T>
T& ArrayRef<T>::operator[](size_t index) {
    return m_data_[index];
}

template <typename T>
T& ArrayRef<T>::operator[](size_t index) const {
    return m_data_[index];
}

template <typename T>
size_t ArrayRef<T>::size() const {
    return m_size_;
}

template <typename T>
T* ArrayRef<T>::data() const {
    return m_data_;
}

template <typename T>
bool ArrayRef<T>::is_empty() const {
    return m_size_ == 0;
}
