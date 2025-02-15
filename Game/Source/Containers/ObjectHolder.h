#pragma once

#include <cstdint>
#include <memory>

#include "common.h"

template <typename T>
class ObjectHolder {
    alignas(T) u8 m_data_[sizeof(T)]{};
    bool m_initialized_{false};
public:
    ObjectHolder() = default;
    template <typename... Args>
    ObjectHolder(Args&&... args);
    ~ObjectHolder();
    ObjectHolder(const ObjectHolder&) = delete;
    ObjectHolder& operator=(const ObjectHolder&) = delete;
    ObjectHolder(ObjectHolder&&) noexcept;
    ObjectHolder& operator=(ObjectHolder&&) noexcept;

    T* get() {
        return reinterpret_cast<T*>(m_data_);
    }
    
    [[nodiscard]] const T* get() const {
        return reinterpret_cast<const T*>(m_data_);
    }

    T& operator*() {
        return *get();
    }

    const T& operator*() const {
        return *get();
    }

    T* operator->() {
        return get();
    }

    const T* operator->() const {
        return get();
    }

    void reset() noexcept {
        if (m_initialized_) {
            get()->~T();
        }
        m_initialized_ = false;
    }
    
    template <typename ...Args>
    void emplace(Args&&... args) {
        reset();
        new (m_data_) T(std::forward<Args>(args)...);
        m_initialized_ = true;
    }
};

template <typename T>
template <typename ... Args>
ObjectHolder<T>::ObjectHolder(Args&&... args) {
    emplace(std::forward<Args>(args)...);
}

template <typename T>
ObjectHolder<T>::~ObjectHolder() {
    reset();
}

template <typename T>
ObjectHolder<T>::ObjectHolder(ObjectHolder&& other) noexcept {
    m_initialized_ = other.m_initialized_;
    other.m_initialized_ = false;
    if (m_initialized_) {
        memcpy(m_data_, other.m_data_, sizeof(T));
    }
}

template <typename T>
ObjectHolder<T>& ObjectHolder<T>::operator=(ObjectHolder&& other) noexcept {
    if (this != &other) {
        m_initialized_ = other.m_initialized_;
        other.m_initialized_ = false;
        if (m_initialized_) {
            memcpy(m_data_, other.m_data_, sizeof(T));
        }
    }
    return *this;
}
