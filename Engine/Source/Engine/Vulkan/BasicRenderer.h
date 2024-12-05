#pragma once
#include <memory>

#include "Engine/Containers/ObjectHolder.h"
#include "Wrappers/CommandBufferWrapper.h"
#include "Wrappers/DeviceWrapper.h"
#include "Wrappers/SwapChain.h"

namespace engine::vulkan {

class BasicRenderer {
    allocators::StackAllocator& m_allocator_;
    void* m_swap_chain_mem_buffer_;
    Window* m_window_;
    DeviceWrapper* m_device_;
    VkSurfaceKHR m_surface_;
    ObjectHolder<SwapChain> m_swap_chain_;
    CommandBufferWrapper m_command_buffer_;

    uint32_t m_current_frame_ = 0;
    uint32_t m_current_image_index_ = 0;
    bool m_is_frame_in_progress_ = false;
public:
    BasicRenderer(allocators::StackAllocator& allocator, Window* window, DeviceWrapper* device, VkSurfaceKHR surface);
    BasicRenderer(const BasicRenderer&) = delete;
    BasicRenderer(BasicRenderer&&) = delete;
    BasicRenderer& operator=(const BasicRenderer&) = delete;
    BasicRenderer& operator=(BasicRenderer&&) = delete;
    ~BasicRenderer() = default;

    VkCommandBuffer begin_frame();
    void end_frame();
    void begin_render_pass(VkCommandBuffer command_buffer);
    void end_render_pass(VkCommandBuffer command_buffer) const;

    [[nodiscard]] bool is_frame_in_progress() const;
    [[nodiscard]] VkCommandBuffer get_current_cmd_buffer() const;
    [[nodiscard]] VkRenderPass get_render_pass() const;

    [[nodiscard]] VkExtent2D get_swap_chain_extent() const;
    
    ObjectHolder<SwapChain> initialize_swap_chain();
    void recreate_swap_chain();
};

}
