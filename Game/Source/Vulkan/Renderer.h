#pragma once

#include <utils.hpp>
#include <vuk/Context.hpp>
#include <vuk/resources/DeviceFrameResource.hpp>
#include <vuk/SampledImage.hpp>

#include "../../Vendor/vk-bootstrap/VkBootstrap.h"
#include "Containers/ObjectHolder.h"
#include "Wrappers/Window.h"

namespace engine {

struct VulkanHandles {
    vkb::Instance instance;
    VkSurfaceKHR surface;
    vkb::Device device;
    vkb::PhysicalDevice physical_device;
    VkQueue graphics_queue;
    VkQueue transfer_queue;
};

class Renderer {
public:
    Window window;
    VulkanHandles handle_struct;
    ObjectHolder<vuk::Context> context;
    ObjectHolder<vuk::DeviceSuperFrameResource> superframe_resource;
    ObjectHolder<vuk::Allocator> superframe_allocator;
    vuk::SwapchainRef swap_chain;
    util::ImGuiData imgui_data;
    vuk::Unique<std::array<VkSemaphore, 3>> present_ready_semaphores;
    vuk::Unique<std::array<VkSemaphore, 3>> render_complete_semaphores;
    plf::colony<vuk::SampledImage> sampled_images;
    vuk::SingleSwapchainRenderBundle bundle;
    
    bool is_suspended = false;

    Renderer();
    ~Renderer() = default;

    void render();
};

}
