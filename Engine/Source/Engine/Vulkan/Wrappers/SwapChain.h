#pragma once
#include "DeviceWrapper.h"
#include "SurfaceWrapper.h"
#include "Containers/ArrayRef.h"
#include "MemoryArena/Arena.h"

namespace engine::vulkan {

class SwapChain {
public:
    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        ArrayRef<VkSurfaceFormatKHR> formats;
        ArrayRef<VkPresentModeKHR> present_modes;
    };
private:
    uint32_t* m_full_buffer_start_;
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
    SwapChain(void* array_buffer, Arena& temp_arena, Arena& permanent_arena, GLFWwindow* window, VkSurfaceKHR surface, DeviceWrapper* device);
    ~SwapChain();

    void create_swap_chain(Arena& temp_arena);
    void create_swap_chain_images(Arena& permanent_arena);
    void create_swap_chain_image_views();
    void create_frame_buffers(VkRenderPass render_pass);
    size_t get_image_views_count() const;
    const ArrayRef<VkImageView>& get_image_views() const;

    VkExtent2D get_swap_chain_extent() const;
    VkFormat get_swap_chain_format() const;
    VkFramebuffer get_frame_buffer(uint32_t index) const;

    VkRenderPass create_render_pass() const;
    VkRenderPass get_current_render_pass() const;

    uint32_t* get_starting_stack_pos() const;

    operator VkSwapchainKHR() const;
    
    static VkSurfaceFormatKHR choose_swap_surface_format(const ArrayRef<VkSurfaceFormatKHR>& available_formats);
    static VkPresentModeKHR choose_present_mode(const ArrayRef<VkPresentModeKHR>& available_present_modes);
    static VkExtent2D choose_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
    static SupportDetails create_support_details(Arena& arena, VkSurfaceKHR surface, VkPhysicalDevice physical_device);
};


}