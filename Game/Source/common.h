#pragma once
#include <cstdint>
#include "Logging/Logger.h"
#include "Memory/STLArenaAllocator.h"

using byte = uint8_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

using arena_string = std::basic_string<char, std::char_traits<char>, STLArenaAllocator<char>>;

template <typename T>
using arena_vector = std::vector<T, STLArenaAllocator<T>>;

#define MAKE_ARENA_VECTOR(arena_ptr, T) arena_vector<T>{STLArenaAllocator<T>{arena_ptr}}

template <typename K, typename V>
using arena_map = std::unordered_map<K, V, std::hash<K>, std::equal_to<K>, STLArenaAllocator<std::pair<const K, V>>>;

#define MAKE_ARENA_MAP(arena_ptr, K, V) arena_map<K, V>{STLArenaAllocator<std::pair<const K, V>>{arena_ptr}}

#ifdef DEBUG
#define VULKAN_ASSERT(x, format_msg, ...) if ((x) != VK_SUCCESS) \
    {ENGINE_LOG_ERROR(format_msg __VA_OPT__(,) __VA_ARGS__) \
    __debugbreak();\
    }
#else
#define VULKAN_ASSERT(x, format_msg, ...) if ((x) != VK_SUCCESS) {\
    ENGINE_LOG_ERROR(format_msg __VA_OPT__(,) __VA_ARGS__) \
    assert(false);}
#endif

#define ENGINE_ASSERT(x, format_msg, ...) if (!(x)) {\
    ENGINE_LOG_ERROR(format_msg __VA_OPT__(,) __VA_ARGS__) \
    assert(false);}
