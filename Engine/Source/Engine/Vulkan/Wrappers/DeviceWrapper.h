#pragma once

#include "QueueWrapper.h"
#include "SurfaceWrapper.h"
#include "Engine/Containers/DynArray.h"

namespace engine::vulkan {

class DeviceWrapper {
    VkDevice m_device_;
    VkPhysicalDevice m_physical_device_;
    QueueWrapper m_graphics_queue_;
    QueueWrapper m_present_queue_;
    QueueWrapper::QueueFamily m_graphics_queue_family_;
public:
    DeviceWrapper(VkSurfaceKHR surface, VkInstance instance, const DynArray<const char*>& validation_layers);
    ~DeviceWrapper();

    operator VkDevice() const;

    VkDevice get_logical_device() const;
    VkPhysicalDevice get_physical_device() const;
    QueueWrapper get_graphics_queue() const;
    QueueWrapper get_present_queue() const;
    QueueWrapper::QueueFamily get_graphics_queue_family() const;
};

}
