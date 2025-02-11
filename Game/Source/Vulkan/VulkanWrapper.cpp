#include "VulkanWrapper.h"

namespace engine::vulkan {

VulkanWrapper::VulkanWrapper(Arena& temp_arena) : 
    window_(1000, 800, "Vulkan Window"),
    m_instance_(temp_arena, true),
    m_validation_layers_(m_instance_.enabled_validation_layers()),
    m_surface_(m_instance_, window_.raw_window()),
    m_device_(temp_arena, m_surface_, m_instance_, m_validation_layers_),
    m_descriptor_pool_(m_device_)
    {
    
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

InstanceWrapper& VulkanWrapper::instance() {
    return m_instance_;
}

DescriptorPool& VulkanWrapper::descriptor_pool() {
    return m_descriptor_pool_;
}

}