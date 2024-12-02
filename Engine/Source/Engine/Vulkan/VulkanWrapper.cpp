#include "VulkanWrapper.h"

namespace engine::vulkan {

VulkanWrapper::VulkanWrapper() : window_(1000, 800, "Vulkan Window"),
    m_instance_(true),
    m_surface_(m_instance_, window_.raw_window()),
    m_device_(m_surface_, m_instance_, m_instance_.enabled_validation_layers()),
    m_swap_chain_(window_.raw_window(), m_surface_, &m_device_),
    m_pipeline_(&m_swap_chain_, &m_device_),
    m_command_buffer_(&m_device_, &m_pipeline_, &m_swap_chain_, m_device_.get_graphics_queue_family().graphics_family_index) {
    
}

Window& VulkanWrapper::window() {
    return window_;
}

void VulkanWrapper::draw_frame() {
    uint32_t image_index;
    vkAcquireNextImageKHR(m_device_, m_swap_chain_, UINT64_MAX, m_command_buffer_.get_image_available_semaphore(), VK_NULL_HANDLE, &image_index);
    m_command_buffer_.reset_command_buffer();
    m_command_buffer_.record_command_buffer(m_pipeline_.get_render_pass(), image_index);
    
}

}