#pragma once
#include "DeviceWrapper.h"
#include "SurfaceWrapper.h"

namespace engine::vulkan {

class SwapChain {
public:
    using SurfaceFormatAlloc = allocators::StackAllocator<void>::rebind<VkSurfaceFormatKHR>::other;
    using PresentModeAlloc = allocators::StackAllocator<void>::rebind<VkPresentModeKHR>::other;
    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        allocators::stack_vec<VkSurfaceFormatKHR> formats;
        allocators::stack_vec<VkPresentModeKHR> present_modes;
    };
private:
    allocators::StackAllocator<void>* m_temp_allocator_;
    uint8_t* m_full_buffer_start_;
    GLFWwindow* m_window_;
    DeviceWrapper* m_device_;
    VkSurfaceKHR m_surface_;
    VkSwapchainKHR m_swap_chain_;
    VkRenderPass m_render_pass_;
    
    uint32_t m_image_count_;
    ArrayRef<VkImage> m_images_;
    ArrayRef<VkImageView> m_image_views_;
    ArrayRef<VkFramebuffer> m_frame_buffers_;
    VkFormat m_swap_chain_format_;
    VkExtent2D m_swap_chain_extent_;
public:
    SwapChain(void* array_buffer, allocators::StackAllocator<void>* temp_allocator, allocators::StackAllocator<void>* setup_allocator, GLFWwindow* window, VkSurfaceKHR surface, DeviceWrapper* device);
    SwapChain(const SwapChain& other) = delete;
    SwapChain& operator=(const SwapChain& other) = delete;
    SwapChain(SwapChain&& swap_chain) = delete;
    SwapChain& operator=(SwapChain&& swap_chain) = delete;
    ~SwapChain();

    void create_swap_chain();
    void create_swap_chain_images(allocators::StackAllocator<void>& setup_allocator);
    void create_swap_chain_image_views();
    void create_frame_buffers(VkRenderPass render_pass);
    size_t get_image_views_count() const;
    const ArrayRef<VkImageView>& get_image_views() const;

    VkExtent2D get_swap_chain_extent() const;
    VkFormat get_swap_chain_format() const;
    VkFramebuffer get_frame_buffer(uint32_t index) const;

    VkRenderPass create_render_pass() const;
    VkRenderPass get_current_render_pass() const;

    uint8_t* get_starting_stack_pos() const;

    operator VkSwapchainKHR() const;
    
    static VkSurfaceFormatKHR choose_swap_surface_format(const allocators::stack_vec<VkSurfaceFormatKHR>& available_formats);
    static VkPresentModeKHR choose_present_mode(const allocators::stack_vec<VkPresentModeKHR>& available_present_modes);
    static VkExtent2D choose_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
    static SupportDetails create_support_details(const allocators::StackAllocator<void>& temp_allocator, VkSurfaceKHR surface, VkPhysicalDevice physical_device);
};


}