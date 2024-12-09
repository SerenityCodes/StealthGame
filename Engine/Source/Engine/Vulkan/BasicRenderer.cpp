#include "BasicRenderer.h"

#include <cassert>
#include <stdexcept>

namespace engine::vulkan {

BasicRenderer::BasicRenderer(Arena& temp_arena, Arena& permanent_arena, Window* window, DeviceWrapper* device, VkSurfaceKHR surface) : m_window_(window), m_device_(device),
    m_surface_(surface), m_swap_chain_(initialize_swap_chain(temp_arena, permanent_arena)), m_command_buffer_(device, m_swap_chain_.get()) {
}

VkCommandBuffer BasicRenderer::begin_frame(Arena& temp_arena, Arena& permanent_arena) {
    assert(!m_is_frame_in_progress_ && "Cannot call begin_frame while another is in progress");
    m_command_buffer_.wait_for_fence(m_current_frame_);
    if (auto res = vkAcquireNextImageKHR(*m_device_, *m_swap_chain_, UINT64_MAX, m_command_buffer_.get_image_available_semaphore(m_current_frame_), VK_NULL_HANDLE, &m_current_image_index_); res != VK_SUCCESS) {
        if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR) {
            recreate_swap_chain(temp_arena, permanent_arena);
            return nullptr;
        }
        throw std::runtime_error("failed to acquire swap chain image");
    }
    m_is_frame_in_progress_ = true;
    m_command_buffer_.reset_fence(m_current_frame_);
    m_command_buffer_.reset_command_buffer(m_current_frame_);

    VkCommandBuffer current_cmd_buffer = get_current_cmd_buffer();
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(current_cmd_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    return current_cmd_buffer;
}

void BasicRenderer::end_frame(Arena& temp_arena, Arena& permanent_arena) {
    assert(m_is_frame_in_progress_ && "Can't end frame when there's no frame to end");
    VkCommandBuffer current_cmd_buffer = get_current_cmd_buffer();
    if (vkEndCommandBuffer(current_cmd_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
    m_command_buffer_.submit_command_buffer(m_current_frame_);
    if (m_command_buffer_.present_command_buffer(m_current_frame_, m_current_image_index_) || m_window_->was_resized()) {
        m_window_->toggle_resized();
        recreate_swap_chain(temp_arena, permanent_arena);
    }
    m_is_frame_in_progress_ = false;
    m_current_frame_ = (m_current_frame_ + 1) % 2;
}

void BasicRenderer::begin_render_pass(VkCommandBuffer command_buffer) {
    assert(m_is_frame_in_progress_ && "Can't end frame when there's no frame to end");
    assert(command_buffer == get_current_cmd_buffer() && "Can't begin render pass on command buffer from a different frame");
    VkCommandBuffer current_cmd_buffer = m_command_buffer_.get_command_buffer(m_current_frame_);
    
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_swap_chain_->get_current_render_pass();
    render_pass_info.framebuffer = m_swap_chain_->get_frame_buffer(m_current_image_index_);
    assert(render_pass_info.framebuffer != VK_NULL_HANDLE && "Framebuffer is invalid!");
    render_pass_info.renderArea = {{0, 0}, m_swap_chain_->get_swap_chain_extent()};

    VkClearValue clear_value = {{{0.1f, 0.1f, 0.1f, 1.0}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(current_cmd_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swap_chain_->get_swap_chain_extent().width);
    viewport.height = static_cast<float>(m_swap_chain_->get_swap_chain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(current_cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = m_swap_chain_->get_swap_chain_extent();
    vkCmdSetScissor(current_cmd_buffer, 0, 1, &scissor);
}

void BasicRenderer::end_render_pass(VkCommandBuffer command_buffer) const {
    assert(m_is_frame_in_progress_ && "Can't end render pass when there's no frame to end");
    assert(command_buffer == get_current_cmd_buffer() && "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(command_buffer);
}

bool BasicRenderer::is_frame_in_progress() const {
    return m_is_frame_in_progress_;
}

VkCommandBuffer BasicRenderer::get_current_cmd_buffer() const {
    return m_command_buffer_.get_command_buffer(m_current_frame_);
}

VkRenderPass BasicRenderer::get_render_pass() const {
    return m_swap_chain_->get_current_render_pass();
}

VkExtent2D BasicRenderer::get_swap_chain_extent() const {
    return m_swap_chain_->get_swap_chain_extent();
}

float BasicRenderer::get_aspect_ratio() const {
    VkExtent2D extent = m_swap_chain_->get_swap_chain_extent();
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}

void BasicRenderer::bind_pipeline(VkPipeline pipeline) {
    m_command_buffer_.bind_command_buffer(pipeline, m_current_frame_);
}

ObjectHolder<SwapChain> BasicRenderer::initialize_swap_chain(Arena& temp_arena, Arena& permanent_arena) {
    vkDeviceWaitIdle(*m_device_);
    ObjectHolder<SwapChain> new_swap_chain{nullptr, temp_arena, permanent_arena, m_window_->raw_window(), m_surface_, m_device_};
    return new_swap_chain;
}

void BasicRenderer::recreate_swap_chain(Arena& temp_arena, Arena& permanent_arena) {
    vkDeviceWaitIdle(*m_device_);
    m_swap_chain_.emplace(m_swap_chain_->get_starting_stack_pos(), temp_arena, permanent_arena, m_window_->raw_window(), m_surface_, m_device_);
}

}