#pragma once

#include <vulkan/vulkan.h>

#include <array>

#include "Engine/Containers/ArrayRef.h"
#include "Engine/Containers/DynArray.h"

namespace engine::vulkan {

class InstanceWrapper {
    VkInstance m_instance_;
    std::array<const char*, 1> m_enabled_validation_layers_;
    allocators::StackAllocator& m_allocator_;
public:
    InstanceWrapper(allocators::StackAllocator& allocator, bool enable_validation_layers);
    InstanceWrapper(const InstanceWrapper&) = delete;
    InstanceWrapper(InstanceWrapper&&) = delete;
    InstanceWrapper& operator=(const InstanceWrapper&) = delete;
    InstanceWrapper& operator=(InstanceWrapper&&) = delete;
    ~InstanceWrapper();
    
    operator VkInstance() const;
    const std::array<const char*, 1>& enabled_validation_layers() const;
};

}
