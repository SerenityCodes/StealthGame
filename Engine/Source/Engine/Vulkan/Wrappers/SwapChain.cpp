﻿#include "SwapChain.h"

#include <stdexcept>

namespace engine::vulkan {

SwapChain::SwapChain(void* array_buffer, allocators::StackAllocator& allocator, GLFWwindow* window, VkSurfaceKHR surface, DeviceWrapper* device) : m_allocator_(allocator), m_full_buffer_start_(static_cast<uint32_t*>(array_buffer)), m_window_(window), m_device_(device), m_surface_(surface),
    m_swap_chain_(nullptr), m_render_pass_(nullptr) {
    create_swap_chain();
    create_swap_chain_images();
    create_swap_chain_image_views();
    m_render_pass_ = create_render_pass();
    create_frame_buffers(m_render_pass_);
}

SwapChain::~SwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(m_window_, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(m_window_, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(*m_device_);
    vkDestroyRenderPass(*m_device_, m_render_pass_, nullptr);
    for (const auto& frame_buffer : m_frame_buffers_) {
        vkDestroyFramebuffer(m_device_->get_logical_device(), frame_buffer, nullptr);
    }
    for (const auto& image_view : m_image_views_) {
        vkDestroyImageView(m_device_->get_logical_device(), image_view, nullptr);
    }
    vkDestroySwapchainKHR(m_device_->get_logical_device(), m_swap_chain_, nullptr);
}

void SwapChain::create_swap_chain() {
    size_t temp_bytes_allocated = 0;
    SupportDetails support_details = create_support_details(m_allocator_, m_surface_, m_device_->get_physical_device(), temp_bytes_allocated);
    m_allocator_.free_bytes(temp_bytes_allocated);
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

    QueueWrapper::QueueFamily family = QueueWrapper::find_indices(m_allocator_, m_surface_, m_device_->get_physical_device());
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

    if (auto res = vkCreateSwapchainKHR(*m_device_, &swap_chain_create_info, nullptr, &m_swap_chain_); res != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }
    m_swap_chain_format_ = surface_format.format;
    m_swap_chain_extent_ = swap_chain_extent;
}

void SwapChain::create_swap_chain_images() {
    vkGetSwapchainImagesKHR(m_device_->get_logical_device(), m_swap_chain_, &m_image_count_, nullptr);
    if (m_full_buffer_start_ == nullptr) {
        size_t bytes_for_all_images = sizeof(VkImage) * m_image_count_ + sizeof(VkImageView) * m_image_count_ + sizeof(VkFramebuffer) * m_image_count_;
        m_full_buffer_start_ = static_cast<uint32_t*>(m_allocator_.allocate_raw(bytes_for_all_images));
    }
    VkImage* arr_ptr = reinterpret_cast<VkImage*>(m_full_buffer_start_);
    m_images_ = ArrayRef(arr_ptr, m_image_count_);
    vkGetSwapchainImagesKHR(m_device_->get_logical_device(), m_swap_chain_, &m_image_count_, m_images_.data());
}

void SwapChain::create_swap_chain_image_views() {
    size_t buffer_offset = m_image_count_ * sizeof(VkImage);
    auto arr_ptr = m_full_buffer_start_ + buffer_offset;
    m_image_views_ = ArrayRef(reinterpret_cast<VkImageView*>(arr_ptr), m_image_count_);

    for (uint32_t i = 0; i < m_images_.size(); i++) {
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
    size_t buffer_offset = m_image_count_ * sizeof(VkImage) + sizeof(VkImageView) * m_image_count_;
    auto arr_ptr = m_full_buffer_start_ + buffer_offset;
    m_frame_buffers_ = ArrayRef(reinterpret_cast<VkFramebuffer*>(arr_ptr), m_image_count_);
    for (uint32_t i = 0; i < m_frame_buffers_.size(); i++) {
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
    return m_images_.size();
}

const ArrayRef<VkImageView>& SwapChain::get_image_views() const {
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

VkRenderPass SwapChain::create_render_pass() const {
    VkAttachmentDescription color_attachment{};
    color_attachment.format = m_swap_chain_format_;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkRenderPassCreateInfo render_pass_create_info{};
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &dependency;

    VkRenderPass render_pass = VK_NULL_HANDLE;
    if (vkCreateRenderPass(*m_device_, &render_pass_create_info, nullptr, &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
    return render_pass;
}

VkRenderPass SwapChain::get_current_render_pass() const {
    return m_render_pass_;
}

uint32_t* SwapChain::get_starting_stack_pos() const {
    return static_cast<uint32_t*>(m_full_buffer_start_);
}

SwapChain::operator VkSwapchainKHR() const {
    return m_swap_chain_;
}

VkSurfaceFormatKHR SwapChain::choose_swap_surface_format(
    const ArrayRef<VkSurfaceFormatKHR>& available_formats) {
    for (const auto& available_format : available_formats) {
        if (available_format.format == VK_FORMAT_B8G8R8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return available_format;
        }
    }
    return available_formats[0];
}

VkPresentModeKHR SwapChain::choose_present_mode(
    const ArrayRef<VkPresentModeKHR>& available_present_modes) {
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
    VkSurfaceCapabilitiesKHR capabilities_khr{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities_khr);
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    VkSurfaceFormatKHR* surface_format_ptr = nullptr;
    if (format_count != 0) {
        surface_format_ptr = new VkSurfaceFormatKHR[format_count];
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_format_ptr);
    }
    VkPresentModeKHR* present_modes_ptr = nullptr;
    if (present_mode_count != 0) {
        present_modes_ptr = new VkPresentModeKHR[present_mode_count];
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes_ptr);
    }
    return {.capabilities = capabilities_khr,
        .formats = ArrayRef{surface_format_ptr, format_count, true},
        .present_modes = ArrayRef{present_modes_ptr, present_mode_count, true}};
}

SwapChain::SupportDetails SwapChain::create_support_details(
    allocators::StackAllocator& allocator, VkSurfaceKHR surface,
    VkPhysicalDevice physical_device, size_t& bytes_allocated) {
    VkSurfaceCapabilitiesKHR capabilities_khr;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities_khr);
    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    uint32_t present_mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
    size_t surface_format_bytes = sizeof(VkSurfaceFormatKHR) * format_count;
    size_t surface_present_mode_bytes = sizeof(VkPresentModeKHR) * present_mode_count;
    size_t total_alloc_size = surface_format_bytes + surface_present_mode_bytes;
    bytes_allocated = total_alloc_size;
    VkSurfaceFormatKHR* formats = static_cast<VkSurfaceFormatKHR*>(allocator.allocate_raw(surface_format_bytes));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats);
    VkPresentModeKHR* present_modes = static_cast<VkPresentModeKHR*>(allocator.allocate_raw(surface_present_mode_bytes));
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);
    return {.capabilities = capabilities_khr,
        .formats = ArrayRef{formats, format_count},
        .present_modes = ArrayRef{present_modes, present_mode_count}};
}

}