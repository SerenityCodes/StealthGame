#include "Renderer.h"

#include <shaderc/shaderc.hpp>
#include <vuk/Future.hpp>
#include <vuk/RenderGraph.hpp>

#include "VkBootstrap.h"
#include "backends/imgui_impl_glfw.h"

namespace engine {

Renderer::Renderer() : window(1200, 800, "Game") {
    vkb::InstanceBuilder instance_builder;
    instance_builder.request_validation_layers()
        .set_app_name("Stealth Game")
        .set_engine_name("Stealth Engine")
        .set_engine_version(VK_MAKE_VERSION(1, 0, 0))
        .require_api_version(1, 3, 0);
    vkb::Result<vkb::Instance> instance_return = instance_builder.build();
    ENGINE_ASSERT(instance_return.has_value(), "Failed to create Vulkan instance")
    handle_struct.instance = instance_return.value();

    VULKAN_ASSERT(glfwCreateWindowSurface(handle_struct.instance, window, nullptr, &handle_struct.surface), "Failed to create GLFW window surface")
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        engine::Renderer* renderer = static_cast<engine::Renderer*>(glfwGetWindowUserPointer(window));
        if (width == 0 || height == 0) {
            renderer->is_suspended = true;
        } else {
            renderer->superframe_allocator->deallocate(std::span{&renderer->swap_chain->swapchain, 1});
            renderer->superframe_allocator->deallocate(renderer->swap_chain->image_views);
            renderer->context->remove_swapchain(renderer->swap_chain);
            renderer->swap_chain = renderer->context->add_swapchain(util::make_swapchain(renderer->handle_struct.device, renderer->swap_chain->swapchain));
            for (auto& image_view : renderer->swap_chain->image_views) {
                renderer->context->set_name(image_view.payload, "Swapchain Image View");
            }
            renderer->is_suspended = false;
        }
    });

    vkb::PhysicalDeviceSelector device_selector{handle_struct.instance};
    device_selector.set_surface(handle_struct.surface)
        .set_minimum_version(1, 3)
        .add_required_extension(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME)
        .add_required_extension(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME)
        .add_required_extension(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME)
        .add_required_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME)
        .add_required_extension(VK_EXT_CALIBRATED_TIMESTAMPS_EXTENSION_NAME);
    auto phys_ret = device_selector.select();
    ENGINE_ASSERT(phys_ret, "Failed to find a suitable GPU")
    handle_struct.physical_device = phys_ret.value();
    
    vkb::DeviceBuilder device_builder{handle_struct.physical_device};
    VkPhysicalDeviceVulkan12Features vk12features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
    vk12features.timelineSemaphore = true;
    vk12features.descriptorBindingPartiallyBound = true;
    vk12features.descriptorBindingUpdateUnusedWhilePending = true;
    vk12features.shaderSampledImageArrayNonUniformIndexing = true;
    vk12features.runtimeDescriptorArray = true;
    vk12features.descriptorBindingVariableDescriptorCount = true;
    vk12features.hostQueryReset = true;
    vk12features.bufferDeviceAddress = true;
    vk12features.shaderOutputLayer = true;
    VkPhysicalDeviceVulkan11Features vk11features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
    vk11features.shaderDrawParameters = true;
    VkPhysicalDeviceFeatures2 vk10features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR };
    vk10features.features.shaderInt64 = true;
    VkPhysicalDeviceSynchronization2FeaturesKHR sync_feat{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR, .pNext = nullptr, .synchronization2 = true};
    auto device_result = device_builder.add_pNext(&vk12features).add_pNext(&vk11features).add_pNext(&vk10features).add_pNext(&sync_feat).build();
    ENGINE_ASSERT(device_result.has_value(), "Failed to create Vulkan device")
    handle_struct.device = device_result.value();

    handle_struct.graphics_queue = handle_struct.device.get_queue(vkb::QueueType::graphics).value();
    handle_struct.transfer_queue = handle_struct.device.get_queue(vkb::QueueType::transfer).value();

    vuk::ContextCreateParameters::FunctionPointers vk_func_ptrs;
    vk_func_ptrs.vkGetInstanceProcAddr = handle_struct.instance.fp_vkGetInstanceProcAddr;
    vk_func_ptrs.vkGetDeviceProcAddr = handle_struct.device.fp_vkGetDeviceProcAddr;
    context.emplace(vuk::ContextCreateParameters{
        .instance = handle_struct.instance,
        .device = handle_struct.device,
        .physical_device = handle_struct.physical_device,
        .graphics_queue = handle_struct.graphics_queue,
        .graphics_queue_family_index = handle_struct.device.get_queue_index(vkb::QueueType::graphics).value(),
        .compute_queue = VK_NULL_HANDLE,
        .compute_queue_family_index = VK_QUEUE_FAMILY_IGNORED,
        .transfer_queue = handle_struct.transfer_queue,
        .transfer_queue_family_index = handle_struct.device.get_queue_index(vkb::QueueType::transfer).value(),
        .pointers = vk_func_ptrs
    });
    constexpr u32 MAX_IN_FLIGHT_FRAMES = 3;
    superframe_resource.emplace(*context, MAX_IN_FLIGHT_FRAMES);
    superframe_allocator.emplace(*superframe_resource);
    swap_chain = context->add_swapchain(util::make_swapchain(handle_struct.device, {}));
    present_ready_semaphores = vuk::Unique<std::array<VkSemaphore, MAX_IN_FLIGHT_FRAMES>>(*superframe_allocator);
    render_complete_semaphores = vuk::Unique<std::array<VkSemaphore, MAX_IN_FLIGHT_FRAMES>>(*superframe_allocator);

    context->set_shader_target_version(VK_API_VERSION_1_3);

    superframe_allocator->allocate_semaphores(*present_ready_semaphores);
    superframe_allocator->allocate_semaphores(*render_complete_semaphores);
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(window, true);
    imgui_data = util::ImGui_ImplVuk_Init(*superframe_allocator);
}

void Renderer::render() {
    vuk::Compiler compiler;
    
    while (!window.should_close()) {
        Window::glfw_poll_events();
        while (is_suspended) {
            glfwWaitEvents();
        }
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        auto& frame_resource = superframe_resource->get_next_frame();
        context->next_frame();

        vuk::Allocator frame_allocator{frame_resource};
        std::shared_ptr<vuk::RenderGraph> render_graph = std::make_shared<vuk::RenderGraph>("Main Render Graph");
        vuk::Name attachment_name = "Gameplay";
        render_graph->attach_swapchain("_swp", swap_chain);
        render_graph->clear_image("_swp", attachment_name, vuk::ClearColor{0.0f, 0.0f, 0.8f, 1.0f});

        ImGui::ShowDemoWindow();
        
        ImGui::Render();
        auto fut = util::ImGui_ImplVuk_Render(frame_allocator, vuk::Future{render_graph, attachment_name}, imgui_data, ImGui::GetDrawData(), sampled_images);
        std::shared_ptr present_rg{std::make_shared<vuk::RenderGraph>("Presenter")};
        present_rg->attach_in("_src", fut);
        present_rg->release_for_present("_src");
        auto erg = *compiler.link(std::span{&present_rg, 1}, {});
        bundle = *vuk::acquire_one(*context, swap_chain, (*present_ready_semaphores)[context->get_frame_count() % 3], (*render_complete_semaphores)[context->get_frame_count() % 3]);
        auto result = vuk::execute_submit(frame_allocator, std::move(erg), std::move(bundle)).value();
        vuk::present_to_one(*context, std::move(result));
        sampled_images.clear();
    }
    context->wait_idle();
}

}