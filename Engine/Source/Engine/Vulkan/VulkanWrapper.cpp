#include "VulkanWrapper.h"

namespace engine::vulkan {

VulkanWrapper::VulkanWrapper() : window_(1000, 800, "Vulkan Window"),
    m_instance_(true),
    m_surface_(m_instance_, window_.raw_window()),
    m_device_(m_surface_, m_instance_, m_instance_.enabled_validation_layers())
    {
    m_validation_layers_.clear();
}

Window& VulkanWrapper::window() {
    return window_;
}

DeviceWrapper* VulkanWrapper::device() {
    return &m_device_;
}

VkSurfaceKHR VulkanWrapper::surface() {
    return m_surface_;
}

}