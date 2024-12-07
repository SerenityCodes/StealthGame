#include "VulkanWrapper.h"

namespace engine::vulkan {

VulkanWrapper::VulkanWrapper(allocators::StackAllocator& allocator) : m_allocator_(allocator),
    window_(1000, 800, "Vulkan Window"),
    m_instance_(m_allocator_, true),
    m_validation_layers_(m_instance_.enabled_validation_layers()),
    m_surface_(m_instance_, window_.raw_window()),
    m_device_(m_allocator_, m_surface_, m_instance_, m_validation_layers_)
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

}