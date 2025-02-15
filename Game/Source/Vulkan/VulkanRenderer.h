#pragma once

#include "common.h"
#include "glm/glm.hpp"
#include "Containers/ObjectHolder.h"
#include "Wrappers/DeviceBufferWrapper.h"
#include "Wrappers/SwapChain.h"

#include <array>

namespace engine::vulkan {

class DeviceBufferWrapper;

class VulkanRenderer {
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    struct QueueFamily {
        u32 graphics_family_index{std::numeric_limits<u32>::max()};
        u32 present_family_index{std::numeric_limits<u32>::max()};

        [[nodiscard]] bool same_family() const {
            return graphics_family_index == present_family_index;
        }
    };
private:
    Window m_window_;
    VkInstance m_instance_;
    VkSurfaceKHR m_surface_;

#ifdef DEBUG
    std::array<const char*, 1> m_validation_layers_;
#endif
    
    VkPhysicalDevice m_physical_device_;
    VkDevice m_logical_device_;
    VkQueue m_graphics_queue_;
    u32 m_graphics_queue_family_;
    VmaAllocator m_allocator_;
    
    ObjectHolder<SwapChain> m_swap_chain_;

    VkDescriptorPool m_descriptor_pool_;

    VkCommandPool m_command_pool_;
    VkCommandBuffer m_primary_command_buffers_[MAX_FRAMES_IN_FLIGHT]{};
    VkSemaphore m_image_available_semaphores_[MAX_FRAMES_IN_FLIGHT]{};
    VkSemaphore m_render_finished_semaphores_[MAX_FRAMES_IN_FLIGHT]{};
    VkFence m_in_flight_fences_[MAX_FRAMES_IN_FLIGHT]{};

    u64 m_current_frame_{0};
    u32 m_image_index_{0};

#ifdef DEBUG
    bool m_is_frame_in_flight_{false};
#endif
    void reset_command_buffer();

    void recreate_swap_chain(Arena& temp_arena);
public:
    VulkanRenderer(int height, int width, Arena& temp_arena, Arena& permanent_arena);
    ~VulkanRenderer();
    
    VkCommandBuffer begin_frame(Arena& temp_arena);
    void end_frame(Arena& temp_arena);
    void begin_render_pass(VkCommandBuffer command_buffer);
    static void end_render_pass(VkCommandBuffer command_buffer);

    void bind_pipeline(VkPipeline pipeline) const;

    VkCommandBuffer get_one_time_cmd_buffer() const;
    void end_one_time_cmd_buffer(VkCommandBuffer cmd_buffer) const;

    void create_buffer(VkDeviceSize size, VkBufferUsageFlags buffer_flags, VmaAllocationCreateFlags allocation_flags, VkBuffer* buffer, VmaAllocation* allocation) const;
    void copy_buffer(VkBuffer source, VkBuffer dest, VkDeviceSize size) const;

    [[nodiscard]] VkExtent2D get_swap_chain_extent() const;
    [[nodiscard]] float get_aspect_ratio() const;

    bool should_close_window() const;
    void poll();

    GLFWwindow* raw_glfw_window() const;
    VkDevice vulkan_device() const;
    VkPhysicalDevice vulkan_physical_device() const;
    VkRenderPass render_pass() const;
    VkCommandPool command_pool() const;
    VmaAllocator vma_allocator() const;
};

}
