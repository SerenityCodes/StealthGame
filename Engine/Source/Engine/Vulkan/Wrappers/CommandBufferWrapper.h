#pragma once

#include "DeviceWrapper.h"
#include "PipelineWrapper.h"
#include "SwapChain.h"

namespace engine::vulkan {

class CommandBufferWrapper {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
private:
    Window* m_window_;
    DeviceWrapper* m_device_;
    PipelineWrapper* m_pipeline_;
    SwapChain* m_swap_chain_;
    VkCommandPool m_command_pool_;
    
    VkCommandBuffer m_primary_command_buffers_[MAX_FRAMES_IN_FLIGHT]{};
    VkSemaphore m_image_available_semaphores_[MAX_FRAMES_IN_FLIGHT]{};
    VkSemaphore m_render_finished_semaphores_[MAX_FRAMES_IN_FLIGHT]{};
    VkFence m_in_flight_fences_[MAX_FRAMES_IN_FLIGHT]{};
public:
    CommandBufferWrapper(Window* window, DeviceWrapper* device, PipelineWrapper* pipeline, SwapChain* swap_chain, uint32_t graphics_queue_family);
    ~CommandBufferWrapper();

    void wait_for_fence(uint32_t current_frame) const;
    void reset_fence(uint32_t current_frame) const;
    void record_command_buffer(VkRenderPass render_pass, uint32_t image_index, uint32_t current_frame) const;
    void reset_command_buffer(uint32_t current_frame) const;
    void submit_command_buffer(uint32_t current_frame) const;
    [[nodiscard]] bool present_command_buffer(uint32_t current_frame, uint32_t image_index) const;

    void reset_swap_chain_ptr(SwapChain* swap_chain);

    [[nodiscard]] const VkCommandBuffer* get_command_buffer_ptr() const;

    VkSemaphore get_image_available_semaphore(uint32_t current_frame) const;
    VkSemaphore get_render_finished_semaphore(uint32_t current_frame) const;
    VkFence get_in_flight_fence(uint32_t current_frame) const;
};

}
