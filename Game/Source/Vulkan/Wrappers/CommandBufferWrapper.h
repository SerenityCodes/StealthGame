#pragma once

#include "DeviceWrapper.h"
#include "SwapChain.h"

namespace engine::vulkan {

class CommandBufferWrapper {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
private:
    DeviceWrapper* m_device_;
    SwapChain* m_swap_chain_;
    VkCommandPool m_command_pool_;
    
    VkCommandBuffer m_primary_command_buffers_[MAX_FRAMES_IN_FLIGHT]{};
    VkSemaphore m_image_available_semaphores_[MAX_FRAMES_IN_FLIGHT]{};
    VkSemaphore m_render_finished_semaphores_[MAX_FRAMES_IN_FLIGHT]{};
    VkFence m_in_flight_fences_[MAX_FRAMES_IN_FLIGHT]{};
public:
    CommandBufferWrapper() = default;
    CommandBufferWrapper(DeviceWrapper* device, SwapChain* swap_chain);
    ~CommandBufferWrapper();
    
    void wait_for_fence(uint32_t current_frame) const;
    void reset_fence(uint32_t current_frame) const;
    void reset_command_buffer(uint32_t current_frame) const;
    void submit_command_buffer(uint32_t current_frame) const;
    void bind_command_buffer(VkPipeline pipeline, uint32_t current_frame);
    [[nodiscard]] bool present_command_buffer(uint32_t current_frame, uint32_t image_index) const;

    VkCommandBuffer get_command_buffer(uint32_t current_frame) const;
    VkCommandPool get_command_pool() const;

    void reset_swap_chain_ptr(SwapChain* swap_chain);
    
    VkSemaphore get_image_available_semaphore(uint32_t current_frame) const;
    VkSemaphore get_render_finished_semaphore(uint32_t current_frame) const;
    VkFence get_in_flight_fence(uint32_t current_frame) const;
};

}
