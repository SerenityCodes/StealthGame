#pragma once
#include "DeviceWrapper.h"
#include "SurfaceWrapper.h"
#include "Engine/Containers/DynArray.h"

namespace engine::vulkan {

class SwapChain {
public:
    struct SupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        DynArray<VkSurfaceFormatKHR> formats;
        DynArray<VkPresentModeKHR> present_modes;
    };
private:
    GLFWwindow* m_window_;
    DeviceWrapper* m_device_;
    VkSurfaceKHR m_surface_;
    SupportDetails m_support_details_;
    VkSwapchainKHR m_swap_chain_;
    
    DynArray<VkImage> m_images_;
    DynArray<VkImageView> m_image_views_;
    DynArray<VkFramebuffer> m_frame_buffers_;
    VkFormat m_swap_chain_format_;
    VkExtent2D m_swap_chain_extent_;
public:
    SwapChain(GLFWwindow* window, VkSurfaceKHR surface, DeviceWrapper* device);
    ~SwapChain();

    void create_swap_chain();
    void create_swap_chain_images();
    void create_swap_chain_image_views();
    void create_frame_buffers(VkRenderPass render_pass);
    size_t get_image_views_count() const;
    const DynArray<VkImageView>& get_image_views() const;

    VkExtent2D get_swap_chain_extent() const;
    VkFormat get_swap_chain_format() const;
    VkFramebuffer get_frame_buffer(uint32_t index) const;

    operator VkSwapchainKHR() const;
    SupportDetails get_support_details() const;
    
    static VkSurfaceFormatKHR choose_swap_surface_format(const DynArray<VkSurfaceFormatKHR>& available_formats);
    static VkPresentModeKHR choose_present_mode(const DynArray<VkPresentModeKHR>& available_present_modes);
    static VkExtent2D choose_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities);
    static SupportDetails create_support_details(VkSurfaceKHR surface, VkPhysicalDevice physical_device);
};


}