#pragma once

#include <vulkan/vulkan.h>

#include <array>

#include "Memory/Arena.h"

namespace engine::vulkan {

class InstanceWrapper {
    VkInstance m_instance_;
    std::array<const char*, 1> m_enabled_validation_layers_;
public:
    InstanceWrapper(Arena& temp_arena, bool enable_validation_layers);
    InstanceWrapper(const InstanceWrapper&) = delete;
    InstanceWrapper(InstanceWrapper&&) = delete;
    InstanceWrapper& operator=(const InstanceWrapper&) = delete;
    InstanceWrapper& operator=(InstanceWrapper&&) = delete;
    ~InstanceWrapper();
    
    operator VkInstance() const;
    const std::array<const char*, 1>& enabled_validation_layers() const;
};

}
