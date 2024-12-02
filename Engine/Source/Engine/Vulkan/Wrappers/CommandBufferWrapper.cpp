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
    command_buffer_allocate_info.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(*m_device_,&command_buffer_allocate_info, &m_primary_command_buffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(device->get_logical_device(), &semaphore_create_info, nullptr, &m_image_available_semaphore_) != VK_SUCCESS
        || vkCreateSemaphore(device->get_logical_device(), &semaphore_create_info, nullptr, &m_render_finished_semaphore_) != VK_SUCCESS
        || vkCreateFence(device->get_logical_device(), &fence_create_info, nullptr, &m_in_flight_fence_) !=  VK_SUCCESS) {
        throw std::runtime_error("Failed to create fence!");
    }
}

CommandBufferWrapper::~CommandBufferWrapper() {
    vkDestroyFence(*m_device_, m_in_flight_fence_, nullptr);
    vkDestroySemaphore(*m_device_, m_image_available_semaphore_, nullptr);
    vkDestroySemaphore(*m_device_, m_render_finished_semaphore_, nullptr);
    vkDestroyCommandPool(*m_device_, m_command_pool_, nullptr);
}

CommandBufferWrapper::operator VkCommandBuffer() {
    return m_primary_command_buffer_;
}

CommandBufferWrapper::operator VkCommandBuffer() const {
    return m_primary_command_buffer_;
}

void CommandBufferWrapper::wait_for_fence() const {
    vkWaitForFences(*m_device_, 1, &m_in_flight_fence_, VK_TRUE, UINT64_MAX);
}

void CommandBufferWrapper::reset_fence() const {
    vkResetFences(*m_device_, 1, &m_in_flight_fence_);
}

void CommandBufferWrapper::record_command_buffer(VkRenderPass render_pass, uint32_t image_index) const {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(m_primary_command_buffer_, &begin_info) != VK_SUCCESS) {
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

    vkCmdBeginRenderPass(m_primary_command_buffer_, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(m_primary_command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_->get_pipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swap_chain_->get_swap_chain_extent().width);
    viewport.height = static_cast<float>(m_swap_chain_->get_swap_chain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(m_primary_command_buffer_, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swap_chain_->get_swap_chain_extent();
    vkCmdSetScissor(m_primary_command_buffer_, 0, 1, &scissor);

    vkCmdDraw(m_primary_command_buffer_, 3, 1, 0, 0);
    vkCmdEndRenderPass(m_primary_command_buffer_);

    if (vkEndCommandBuffer(m_primary_command_buffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

void CommandBufferWrapper::reset_command_buffer() const {
    vkResetCommandBuffer(m_primary_command_buffer_, 0);
}

void CommandBufferWrapper::submit_command_buffer() const {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {m_image_available_semaphore_};
    constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_primary_command_buffer_;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_render_finished_semaphore_;

    if (vkQueueSubmit(m_device_->get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit command buffer command buffer!");
    }

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    
}

VkSemaphore CommandBufferWrapper::get_image_available_semaphore() const {
    return m_image_available_semaphore_;
}

VkSemaphore CommandBufferWrapper::get_render_finished_semaphore() const {
    return m_render_finished_semaphore_;
}

VkFence CommandBufferWrapper::get_in_flight_fence() const {
    return m_in_flight_fence_;
}


}