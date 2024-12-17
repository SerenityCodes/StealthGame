#include "CommandBufferWrapper.h"
#include "DeviceWrapper.h"
#include "PipelineWrapper.h"
#include "SwapChain.h"

#include <stdexcept>

namespace engine::vulkan {

CommandBufferWrapper::CommandBufferWrapper(DeviceWrapper* device, SwapChain* swap_chain) : m_device_(device), m_swap_chain_(swap_chain) {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = device->get_graphics_queue_family().graphics_family_index;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    if (vkCreateCommandPool(*m_device_,&command_pool_create_info, nullptr, &m_command_pool_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool!");
    }

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = m_command_pool_;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    if (vkAllocateCommandBuffers(*m_device_,&command_buffer_allocate_info, m_primary_command_buffers_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(device->get_logical_device(), &semaphore_create_info, nullptr, &m_image_available_semaphores_[i]) != VK_SUCCESS
        || vkCreateSemaphore(device->get_logical_device(), &semaphore_create_info, nullptr, &m_render_finished_semaphores_[i]) != VK_SUCCESS
        || vkCreateFence(device->get_logical_device(), &fence_create_info, nullptr, &m_in_flight_fences_[i]) !=  VK_SUCCESS) {
            throw std::runtime_error("Failed to create fence!");
        }   
    }
}

CommandBufferWrapper::~CommandBufferWrapper() {
    vkDeviceWaitIdle(*m_device_);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroyFence(*m_device_, m_in_flight_fences_[i], nullptr);
        vkDestroySemaphore(*m_device_, m_image_available_semaphores_[i], nullptr);
        vkDestroySemaphore(*m_device_, m_render_finished_semaphores_[i], nullptr);
    }
    vkDestroyCommandPool(*m_device_, m_command_pool_, nullptr);
}

void CommandBufferWrapper::wait_for_fence(uint32_t current_frame) const {
    vkWaitForFences(*m_device_, 1, &m_in_flight_fences_[current_frame], VK_TRUE, UINT64_MAX);
}

void CommandBufferWrapper::reset_fence(uint32_t current_frame) const {
    vkResetFences(*m_device_, 1, &m_in_flight_fences_[current_frame]);
}

void CommandBufferWrapper::reset_command_buffer(uint32_t current_frame) const {
    vkResetCommandBuffer(m_primary_command_buffers_[current_frame], 0);
}

void CommandBufferWrapper::submit_command_buffer(uint32_t current_frame) const {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {m_image_available_semaphores_[current_frame]};
    constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_primary_command_buffers_[current_frame];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_render_finished_semaphores_[current_frame];

    if (vkQueueSubmit(m_device_->get_graphics_queue(), 1, &submit_info, m_in_flight_fences_[current_frame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit command buffer command buffer!");
    }
}

void CommandBufferWrapper::bind_command_buffer(VkPipeline pipeline, uint32_t current_frame) {
    vkCmdBindPipeline(m_primary_command_buffers_[current_frame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

bool CommandBufferWrapper::present_command_buffer(uint32_t current_frame, uint32_t image_index) const {
    VkSemaphore wait_semaphores[] = {m_render_finished_semaphores_[current_frame]};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = wait_semaphores;

    VkSwapchainKHR swap_chains[] = {*m_swap_chain_};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    if (auto res = vkQueuePresentKHR(m_device_->get_present_queue(), &present_info); res != VK_SUCCESS) {
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            return true;
        }
        throw std::runtime_error("Failed to present swap chain");
    }
    return false;
}

VkCommandBuffer CommandBufferWrapper::get_command_buffer(
    uint32_t current_frame) const {
    return m_primary_command_buffers_[current_frame];
}

VkCommandPool CommandBufferWrapper::get_command_pool() const {
    return m_command_pool_;
}

void CommandBufferWrapper::reset_swap_chain_ptr(SwapChain* swap_chain) {
    m_swap_chain_ = swap_chain;
}

VkSemaphore CommandBufferWrapper::get_image_available_semaphore(uint32_t current_frame) const {
    return m_image_available_semaphores_[current_frame];
}

VkSemaphore CommandBufferWrapper::get_render_finished_semaphore(uint32_t current_frame) const {
    return m_render_finished_semaphores_[current_frame];
}

VkFence CommandBufferWrapper::get_in_flight_fence(uint32_t current_frame) const {
    return m_in_flight_fences_[current_frame];
}


}