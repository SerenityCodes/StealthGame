#include "CommandBufferWrapper.h"
#include "DeviceWrapper.h"
#include "PipelineWrapper.h"
#include "SwapChain.h"

#include <stdexcept>

namespace engine::vulkan {

CommandBufferWrapper::CommandBufferWrapper(DeviceWrapper* device, PipelineWrapper* pipeline, SwapChain* swap_chain, uint32_t graphics_queue_family) : m_device_(device), m_pipeline_(pipeline), m_swap_chain_(swap_chain) {
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = graphics_queue_family;
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

void CommandBufferWrapper::record_command_buffer(VkRenderPass render_pass, uint32_t image_index, uint32_t current_frame) const {
    VkCommandBuffer current_cmd_buffer = m_primary_command_buffers_[current_frame];
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(current_cmd_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_pass;
    render_pass_info.framebuffer = m_pipeline_->get_frame_buffer(image_index);
    render_pass_info.renderArea = {{0, 0}, m_swap_chain_->get_swap_chain_extent()};

    VkClearValue clear_value = {{{0.0, 0.0, 0.0, 1.0}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(current_cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(current_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_->get_pipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swap_chain_->get_swap_chain_extent().width);
    viewport.height = static_cast<float>(m_swap_chain_->get_swap_chain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(current_cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swap_chain_->get_swap_chain_extent();
    vkCmdSetScissor(current_cmd_buffer, 0, 1, &scissor);

    vkCmdDraw(current_cmd_buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(current_cmd_buffer);

    if (vkEndCommandBuffer(current_cmd_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
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

void CommandBufferWrapper::present_command_buffer(uint32_t current_frame, uint32_t image_index) const {
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

    if (vkQueuePresentKHR(m_device_->get_present_queue(), &present_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain");
    }
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