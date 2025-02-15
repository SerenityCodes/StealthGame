#pragma once
#include <cstdint>
#include "Logging/Logger.h"

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

#ifdef DEBUG
#define VULKAN_ASSERT(x, format_msg, ...) if ((x) != VK_SUCCESS) \
    {ENGINE_LOG_ERROR(format_msg __VA_OPT__(,) __VA_ARGS__) \
    __debugbreak();\
    }
#else
#define CHECK_VULKAN_ERROR_V(x, format_msg, ...) if ((x) != VK_SUCCESS) {\
    ENGINE_LOG_ERROR(format_msg __VA_OPT__(,) __VA_ARGS__) \
    assert(false);}
#endif

#define ENGINE_ASSERT(x, format_msg, ...) if (!(x)) {\
    ENGINE_LOG_ERROR(format_msg __VA_OPT__(,) __VA_ARGS__) \
    assert(false);}
