#pragma once

#include "Engine/Containers/ArrayRef.h"
#include "Wrappers/DeviceWrapper.h"
#include "Wrappers/InstanceWrapper.h"
#include "Wrappers/SurfaceWrapper.h"
#include "Wrappers/Window.h"

namespace engine::vulkan {

class VulkanWrapper {
    allocators::StackAllocator<void>& m_allocator_;
    Window window_;
    InstanceWrapper m_instance_;
    std::array<const char*, 1> m_validation_layers_;
    SurfaceWrapper m_surface_;
    DeviceWrapper m_device_;
public:
    VulkanWrapper(allocators::StackAllocator<void>& allocator);
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
