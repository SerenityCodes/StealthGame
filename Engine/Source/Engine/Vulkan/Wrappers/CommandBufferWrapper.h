#pragma once

#include "DeviceWrapper.h"
#include "PipelineWrapper.h"
#include "SwapChain.h"

namespace engine::vulkan {

class DeviceWrapper;       // Forward declaration
class PipelineWrapper;     // Forward declaration
class SwapChain;           // Forward declaration

class CommandBufferWrapper {
    DeviceWrapper* m_device_;
    PipelineWrapper* m_pipeline_;
    SwapChain* m_swap_chain_;
    VkCommandPool m_command_pool_;
    VkCommandBuffer m_primary_command_buffer_;

    VkSemaphore m_image_available_semaphore_;
    VkSemaphore m_render_finished_semaphore_;
    VkFence m_in_flight_fence_;
public:
    CommandBufferWrapper(DeviceWrapper* device, PipelineWrapper* pipeline, SwapChain* swap_chain, uint32_t graphics_queue_family);
    ~CommandBufferWrapper();

    operator VkCommandBuffer();
    operator VkCommandBuffer() const;

    void wait_for_fence() const;
    void reset_fence() const;
    void record_command_buffer(VkRenderPass render_pass, uint32_t image_index) const;
    void reset_command_buffer() const;
    void submit_command_buffer() const;

    VkSemaphore get_image_available_semaphore() const;
    VkSemaphore get_render_finished_semaphore() const;
    VkFence get_in_flight_fence() const;
};

}
