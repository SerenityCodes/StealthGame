#pragma once

#include "Wrappers/DeviceWrapper.h"
#include "Wrappers/InstanceWrapper.h"
#include "Wrappers/SurfaceWrapper.h"
#include "Wrappers/Window.h"

namespace engine::vulkan {

class VulkanWrapper {
    Window window_;
    DynArray<const char*> m_validation_layers_;
    InstanceWrapper m_instance_;
    SurfaceWrapper m_surface_;
    DeviceWrapper m_device_;
public:
    VulkanWrapper();
    VulkanWrapper(const VulkanWrapper&) = delete;
    VulkanWrapper(VulkanWrapper&&) = delete;
    VulkanWrapper& operator=(const VulkanWrapper&) = delete;
    VulkanWrapper& operator=(VulkanWrapper&&) = delete;
    ~VulkanWrapper() = default;

    Window& window();
    DeviceWrapper* device();
    VkSurfaceKHR surface();
};

}
