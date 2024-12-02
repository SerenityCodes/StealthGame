#pragma once

#include <vulkan/vulkan.h>

#include "Engine/Containers/DynArray.h"

namespace engine::vulkan {

class InstanceWrapper {
    VkInstance m_instance_;
    DynArray<const char*> m_enabled_validation_layers_;
public:
    InstanceWrapper(bool enable_validation_layers);
    InstanceWrapper(const InstanceWrapper&) = delete;
    InstanceWrapper(InstanceWrapper&&) = delete;
    InstanceWrapper& operator=(const InstanceWrapper&) = delete;
    InstanceWrapper& operator=(InstanceWrapper&&) = delete;
    ~InstanceWrapper();
    
    operator VkInstance() const;
    const DynArray<const char*>& enabled_validation_layers() const;
};

}
