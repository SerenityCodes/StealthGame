#include "SwapChain.h"

#include <stdexcept>

namespace engine::vulkan {

SwapChain::SwapChain(GLFWwindow* window, VkSurfaceKHR surface, DeviceWrapper* device) : m_window_(window), m_device_(device), m_surface_(surface),
    m_support_details_(create_support_details(surface, device->get_physical_device())),
    m_swap_chain_(nullptr) {
    create_swap_chain();
    create_swap_chain_images();
    create_swap_chain_image_views();
}

SwapChain::~SwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window_, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window_, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(*m_device_);
    for (const auto& frame_buffer : m_frame_buffers_) {
        vkDestroyFramebuffer(m_device_->get_logical_device(), frame_buffer, nullptr);
    }
    for (const auto& image_view : m_image_views_) {
        vkDestroyImageView(m_device_->get_logical_device(), image_view, nullptr);
    }
    vkDestroySwapchainKHR(m_device_->get_logical_device(), m_swap_chain_, nullptr);
}

void SwapChain::create_swap_chain() {
    SupportDetails support_details = create_support_details(m_surface_, m_device_->get_physical_device());
    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(support_details.formats);
    VkPresentModeKHR present_mode = choose_present_mode(support_details.present_modes);
    VkExtent2D swap_chain_extent = choose_extent(m_window_, support_details.capabilities);

    uint32_t image_count = support_details.capabilities.minImageCount + 1;
    if (support_details.capabilities.maxImageCount > 0 && image_count > support_details.capabilities.maxImageCount) {
        image_count = support_details.capabilities.maxImageCount;
    }
    VkSwapchainCreateInfoKHR swap_chain_create_info{};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = m_surface_;
    swap_chain_create_info.minImageCount = image_count;
    swap_chain_create_info.imageFormat = surface_format.format;
    swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
    swap_chain_create_info.imageExtent = swap_chain_extent;
    swap_chain_create_info.presentMode = present_mode;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueWrapper::QueueFamily family = QueueWrapper::find_indices(m_surface_, m_device_->get_physical_device());
    uint32_t queue_family_indices[] = {family.graphics_family_index, family.present_family_index};
    if (queue_family_indices[0] == queue_family_indices[1]) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_create_info.queueFamilyIndexCount = 0;
        swap_chain_create_info.pQueueFamilyIndices = nullptr;
    } else {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        swap_chain_create_info.pQueueFamilyIndices = queue_family_indices;
    }
    swap_chain_create_info.preTransform = support_details.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(*m_device_, &swap_chain_create_info, nullptr, &m_swap_chain_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }
    m_swap_chain_format_ = surface_format.format;
    m_swap_chain_extent_ = swap_chain_extent;
}

void SwapChain::create_swap_chain_images() {
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(m_device_->get_logical_device(), m_swap_chain_, &image_count, nullptr);
    m_images_.resize(image_count);
    vkGetSwapchainImagesKHR(m_device_->get_logical_device(), m_swap_chain_, &image_count, m_images_.data());
}

void SwapChain::create_swap_chain_image_views() {
    m_image_views_.resize(m_images_.get_size());

    for (uint32_t i = 0; i < m_images_.get_size(); i++) {
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = m_images_[i];
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = m_swap_chain_format_;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_device_->get_logical_device(), &image_view_create_info, nullptr, &m_image_views_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain image view!");
        }
    }
}

void SwapChain::create_frame_buffers(VkRenderPass render_pass) {
    m_frame_buffers_.resize(m_image_views_.get_size());
    for (uint32_t i = 0; i < m_frame_buffers_.get_size(); i++) {
        VkImageView attachments[] = {m_image_views_[i]};
        
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = m_swap_chain_extent_.width;
        framebuffer_create_info.height = m_swap_chain_extent_.height;
        framebuffer_create_info.layers = 1;

        if (vkCreateFramebuffer(*m_device_, &framebuffer_create_info, nullptr, &m_frame_buffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

size_t SwapChain::get_image_views_count() const {
    return m_images_.get_size();
}

const DynArray<VkImageView>& SwapChain::get_image_views() const {
    return m_image_views_;
}

VkExtent2D SwapChain::get_swap_chain_extent() const {
    return m_swap_chain_extent_;
}

VkFormat SwapChain::get_swap_chain_format() const {
    return m_swap_chain_format_;
}

VkFramebuffer SwapChain::get_frame_buffer(uint32_t index) const {
    return m_frame_buffers_[index];
}

SwapChain::operator VkSwapchainKHR() const {
    return m_swap_chain_;
}

SwapChain::SupportDetails SwapChain::get_support_details() const {
    return m_support_details_;
}

VkSurfaceFormatKHR SwapChain::choose_swap_surface_format(
    const DynArray<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR SwapChain::choose_present_mode(
    const DynArray<VkPresentModeKHR>& available_present_modes) {
    for (const auto& available_present_mode : available_present_modes) {
        if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return available_present_mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::choose_extent(GLFWwindow* window, const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actual_extent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actual_extent;
}

SwapChain::SupportDetails SwapChain::create_support_details(VkSurfaceKHR surface, VkPhysicalDevice physical_device) {
    SupportDetails details{.capabilities = {}, .formats = {}, .present_modes = {}};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &details.capabilities);
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    if (format_count != 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, details.formats.data());
    }
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    if (present_mode_count != 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, details.present_modes.data());
    }
    return details;
}

}