#pragma once
#include <vulkan/vulkan_core.h>

#include "DeviceWrapper.h"

namespace engine::vulkan {

class DescriptorPool {
    DeviceWrapper* m_device_;
    VkDescriptorPool m_descriptor_pool_;
public:
    DescriptorPool(DeviceWrapper& device);
    ~DescriptorPool();

    VkDescriptorPool descriptor_pool() const;
};

}
