#include "VulkanWrapper.h"

#include <iostream>
#include <stdexcept>

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
    m_command_buffer_.wait_for_fence(m_current_frame_);
    m_command_buffer_.reset_fence(m_current_frame_);
    uint32_t image_index;
    if (auto res = vkAcquireNextImageKHR(m_device_, m_swap_chain_, UINT64_MAX, m_command_buffer_.get_image_available_semaphore(m_current_frame_), VK_NULL_HANDLE, &image_index); res != VK_SUCCESS) {
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            std::cerr << "Window resized. Moving on...\n";
            return;
        } else {
            throw std::runtime_error("failed to acquire swap chain image");
        }
    }
    m_command_buffer_.reset_command_buffer(m_current_frame_);
    m_command_buffer_.record_command_buffer(m_pipeline_.get_render_pass(), image_index, m_current_frame_);
    m_command_buffer_.submit_command_buffer(m_current_frame_);
    m_command_buffer_.present_command_buffer(m_current_frame_, image_index);
    
    m_current_frame_ = (m_current_frame_ + 1) % CommandBufferWrapper::MAX_FRAMES_IN_FLIGHT;
}

}