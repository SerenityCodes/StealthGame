#include "VulkanRenderer.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "Containers/DynArray.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace engine::vulkan {


bool is_device_suitable(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader && device_features.samplerAnisotropy;
}

bool check_device_extension_support(Arena& temp_arena, VkPhysicalDevice device) {
    const char* needed_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    VkExtensionProperties* arr = static_cast<VkExtensionProperties*>(temp_arena.push(sizeof(VkExtensionProperties) * extension_count));
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, arr);

    auto check_func = [needed_extensions](VkExtensionProperties extension) {
        return strcmp(extension.extensionName, needed_extensions[0]) == 0;
    };
    ArrayRef available_extensions{arr, extension_count};
    return std::any_of(available_extensions.begin(), available_extensions.end(), check_func);
}

bool is_swap_chain_good(Arena& temp_arena, VkSurfaceKHR surface, VkPhysicalDevice device) {
    SwapChain::SupportDetails swap_chain_support_details = SwapChain::create_support_details(temp_arena, surface, device);
    return !swap_chain_support_details.formats.is_empty() && !swap_chain_support_details.present_modes.is_empty();
}

VkPhysicalDevice pick_physical_device(Arena& temp_arena, VkSurfaceKHR surface, VkInstance instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("No suitable GPUs available");
    }
    VkPhysicalDevice* physical_devices_arr = static_cast<VkPhysicalDevice*>(temp_arena.push(sizeof(VkPhysicalDevice) * device_count));
    ArrayRef physical_devices(physical_devices_arr, static_cast<uint16_t>(device_count));
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());
    for (const auto& device : physical_devices) {
        if (is_device_suitable(device) && check_device_extension_support(temp_arena, device) && is_swap_chain_good(temp_arena, surface, device)) {
            return device;
        }
    }
    // Not supposed to make it here
    ENGINE_LOG_ERROR("Failed to find suitable GPU!")
    assert(false);
    return VK_NULL_HANDLE;
}

VulkanRenderer::QueueFamily find_indices(Arena& temp_arena, const VkSurfaceKHR surface, const VkPhysicalDevice physical_device) {
    VulkanRenderer::QueueFamily indices;
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    VkQueueFamilyProperties* queue_families_arr = static_cast<VkQueueFamilyProperties*>(temp_arena.push_zero(queue_family_count * sizeof(VkQueueFamilyProperties)));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families_arr);
    for (uint32_t i = 0; i < queue_family_count; i++) {
        if (queue_families_arr[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family_index = i;
        }
        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if (present_support) {
            indices.present_family_index = i;
        }
        if (indices.graphics_family_index != std::numeric_limits<uint32_t>::max()) {
            break;
        }
    }
    return indices;
}

void check_vulkan_result_func(VkResult res) {
    VULKAN_ASSERT(res, "Failed imgui vulkan result func check")
}

void VulkanRenderer::reset_command_buffer() {
    vkResetCommandBuffer(m_primary_command_buffers_[m_current_frame_], VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

void VulkanRenderer::recreate_swap_chain(Arena& temp_arena) {
    vkDeviceWaitIdle(m_logical_device_);
    // Won't use permanent arena again, so passing in temp_arena for permanent_arena works just fine here
    m_swap_chain_.emplace(m_swap_chain_->get_starting_stack_pos(), temp_arena, temp_arena, m_window_.raw_window(), m_surface_, m_logical_device_, m_physical_device_);
}

VulkanRenderer::VulkanRenderer(int height, int width, Arena& temp_arena, Arena& permanent_arena) : m_window_(height, width, "Stealth Game") {
    // Create the instance
    u32 extension_count = 0;
    const char** needed_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
    DynArray<const char*> extensions_to_enable{temp_arena, extension_count};
    for (u32 i = 0; i < extension_count; i++) {
        extensions_to_enable[i] = needed_extensions[i];
    }
    
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    u32 properties_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, nullptr);
    VkExtensionProperties* properties_ptr = static_cast<VkExtensionProperties*>(temp_arena.push(sizeof(VkExtensionProperties) * properties_count));
    ArrayRef available_extensions{properties_ptr, properties_count};
    vkEnumerateInstanceExtensionProperties(nullptr, &properties_count, available_extensions.data());

    for (const VkExtensionProperties& extension : available_extensions) {
        if (strcmp(extension.extensionName, VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME) == 0) {
            extensions_to_enable.push_back(extension.extensionName);
        }
    }
    
    // Enable validation layers (if needed)
#ifdef DEBUG
    m_validation_layers_ = { "VK_LAYER_KHRONOS_validation" };
    create_info.enabledLayerCount = static_cast<u32>(m_validation_layers_.size());
    create_info.ppEnabledLayerNames = m_validation_layers_.data();
#endif

    create_info.enabledExtensionCount = static_cast<u32>(extensions_to_enable.size());
    create_info.ppEnabledExtensionNames = extensions_to_enable.data();
    VULKAN_ASSERT(vkCreateInstance(&create_info, nullptr, &m_instance_), "Failed to create instance")

    // Create surface
    VULKAN_ASSERT(glfwCreateWindowSurface(m_instance_, m_window_.raw_window(), nullptr, &m_surface_), "Failed to create window surface")
    
    // Create device
    m_physical_device_ = pick_physical_device(temp_arena, m_surface_, m_instance_);
    m_graphics_queue_family_ = find_indices(temp_arena, m_surface_, m_physical_device_).graphics_family_index;
    DynArray device_extensions = {{VK_KHR_SWAPCHAIN_EXTENSION_NAME}, temp_arena};

    constexpr float queue_priority = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = m_graphics_queue_family_;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_priority;
    
    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledExtensionCount = static_cast<u32>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();
    VULKAN_ASSERT(vkCreateDevice(m_physical_device_, &device_create_info, nullptr, &m_logical_device_), "Failed to create logical device")

    vkGetDeviceQueue(m_logical_device_, m_graphics_queue_family_, 0, &m_graphics_queue_);

    
    // Create allocator
    VmaAllocatorCreateInfo allocator_create_info{};
    allocator_create_info.device = m_logical_device_;
    allocator_create_info.instance = m_instance_;
    allocator_create_info.physicalDevice = m_physical_device_;
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_3;
    allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    vmaCreateAllocator(&allocator_create_info, &m_allocator_);

    // Create swap chain
    m_swap_chain_.emplace(nullptr, temp_arena, permanent_arena, m_window_.raw_window(), m_surface_, m_logical_device_, m_physical_device_);

    // Create Descriptor Pool
    VkDescriptorPoolSize descriptor_pool_sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE}};
    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : descriptor_pool_sizes) {
        pool_info.maxSets += pool_size.descriptorCount;
    }
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = descriptor_pool_sizes;
    VULKAN_ASSERT(vkCreateDescriptorPool(m_logical_device_, &pool_info, nullptr, &m_descriptor_pool_), "Failed to create descriptor pool")
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    
    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForVulkan(m_window_.raw_window(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_instance_;
    init_info.PhysicalDevice = m_physical_device_;
    init_info.Device = m_logical_device_;
    init_info.QueueFamily = m_graphics_queue_family_;
    init_info.Queue = m_graphics_queue_;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = m_descriptor_pool_;
    init_info.RenderPass = m_swap_chain_->get_current_render_pass();
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vulkan_result_func;
    ImGui_ImplVulkan_Init(&init_info);

    // Create command buffers
    VkCommandPoolCreateInfo command_pool_create_info{};
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = m_graphics_queue_family_;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    
    VULKAN_ASSERT(vkCreateCommandPool(m_logical_device_, &command_pool_create_info, nullptr, &m_command_pool_), "Failed to create command pool")

    VkCommandBufferAllocateInfo command_buffer_allocate_info{};
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = m_command_pool_;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;
    VULKAN_ASSERT(vkAllocateCommandBuffers(m_logical_device_, &command_buffer_allocate_info, m_primary_command_buffers_), "Failed to allocate command buffers")

    VkSemaphoreCreateInfo semaphore_create_info{};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info{};
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        VULKAN_ASSERT(vkCreateSemaphore(m_logical_device_, &semaphore_create_info, nullptr, &m_image_available_semaphores_[i]), "Failed to create image available semaphore {}", i)
        VULKAN_ASSERT(vkCreateSemaphore(m_logical_device_, &semaphore_create_info, nullptr, &m_render_finished_semaphores_[i]), "Failed to create render semaphore {}", i)
        VULKAN_ASSERT(vkCreateFence(m_logical_device_, &fence_create_info, nullptr, &m_in_flight_fences_[i]), "Failed to create fence {}", i)
    }
}

VulkanRenderer::~VulkanRenderer() {
    vkDeviceWaitIdle(m_logical_device_);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyFence(m_logical_device_, m_in_flight_fences_[i], nullptr);
        vkDestroySemaphore(m_logical_device_, m_image_available_semaphores_[i], nullptr);
        vkDestroySemaphore(m_logical_device_, m_render_finished_semaphores_[i], nullptr);
    }
    vkDestroyCommandPool(m_logical_device_, m_command_pool_, nullptr);
    m_swap_chain_.reset();
    vkDestroyDescriptorPool(m_logical_device_, m_descriptor_pool_, nullptr);
    vmaDestroyAllocator(m_allocator_);
    vkDestroyDevice(m_logical_device_, nullptr);
    vkDestroySurfaceKHR(m_instance_, m_surface_, nullptr);
    vkDestroyInstance(m_instance_, nullptr);
}

VkCommandBuffer VulkanRenderer::begin_frame(Arena& temp_arena) {
#ifdef DEBUG
    ENGINE_ASSERT(!m_is_frame_in_flight_, "Frame cannot be started when frame is in progress. Frame {} is to blame", m_current_frame_)
#endif
    vkWaitForFences(m_logical_device_, 1, &m_in_flight_fences_[m_current_frame_], VK_TRUE, UINT64_MAX);
    VkResult acquire_res = vkAcquireNextImageKHR(m_logical_device_, *m_swap_chain_, UINT64_MAX, m_image_available_semaphores_[m_current_frame_], VK_NULL_HANDLE, &m_image_index_);
    if (acquire_res != VK_SUCCESS) {
        if (acquire_res == VK_ERROR_OUT_OF_DATE_KHR || acquire_res == VK_SUBOPTIMAL_KHR) {
            recreate_swap_chain(temp_arena);
            return nullptr;
        }
        ENGINE_ASSERT(false, "Failed to acquire image from swap chain")
    }
#ifdef DEBUG
    m_is_frame_in_flight_ = true;
#endif
    VkCommandBuffer current_cmd_buffer = m_primary_command_buffers_[m_current_frame_];
    vkResetFences(m_logical_device_, 1, &m_in_flight_fences_[m_current_frame_]);
    vkResetCommandBuffer(current_cmd_buffer, 0);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(current_cmd_buffer, &begin_info) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    return current_cmd_buffer;
}

void VulkanRenderer::end_frame(Arena& temp_arena) {
#ifdef DEBUG
    ENGINE_ASSERT(m_is_frame_in_flight_, "There is no frame to end. Frame {} is to blame", m_current_frame_)
#endif
    // Render ImGUI as a seperate render pass
    VkCommandBuffer current_cmd_buffer = m_primary_command_buffers_[m_current_frame_];
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    ImGui_ImplVulkan_RenderDrawData(draw_data, current_cmd_buffer);
    ImGui::UpdatePlatformWindows();
    VULKAN_ASSERT(vkEndCommandBuffer(current_cmd_buffer), "Failed to end command buffer")

    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {m_image_available_semaphores_[m_current_frame_]};
    constexpr VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_ALL_COMMANDS_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &m_primary_command_buffers_[m_current_frame_];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &m_render_finished_semaphores_[m_current_frame_];

    VULKAN_ASSERT(vkQueueSubmit(m_graphics_queue_, 1, &submit_info, m_in_flight_fences_[m_current_frame_]), "Failed to submit graphics queue")

    VkSemaphore present_wait_semaphores[] = {m_render_finished_semaphores_[m_current_frame_]};
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = present_wait_semaphores;

    VkSwapchainKHR swap_chains[] = {*m_swap_chain_};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &m_image_index_;
    present_info.pResults = nullptr;

    VkResult present_res = vkQueuePresentKHR(m_graphics_queue_, &present_info);
    if (present_res != VK_SUCCESS) {
        if (present_res == VK_ERROR_OUT_OF_DATE_KHR || present_res == VK_SUBOPTIMAL_KHR) {
            m_window_.toggle_resized();
            recreate_swap_chain(temp_arena);
        } else {
            ENGINE_ASSERT(false, "Failed to present swap chain")   
        }
    }
#ifdef DEBUG
    m_is_frame_in_flight_ = false;
#endif
    m_current_frame_ = (m_current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::begin_render_pass(VkCommandBuffer command_buffer) {
    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = m_swap_chain_->get_current_render_pass();
    render_pass_info.framebuffer = m_swap_chain_->get_frame_buffer(m_image_index_);
    ENGINE_ASSERT(render_pass_info.framebuffer != VK_NULL_HANDLE, "Framebuffer is invalid!")
    render_pass_info.renderArea = {{0, 0}, m_swap_chain_->get_swap_chain_extent()};

    VkClearValue clear_value = {{{0.1f, 0.1f, 0.1f, 1.0}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swap_chain_->get_swap_chain_extent().width);
    viewport.height = static_cast<float>(m_swap_chain_->get_swap_chain_extent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = m_swap_chain_->get_swap_chain_extent();
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);
}

void VulkanRenderer::end_render_pass(VkCommandBuffer command_buffer) {
    vkCmdEndRenderPass(command_buffer);
}

void VulkanRenderer::bind_pipeline(VkPipeline pipeline) const {
    vkCmdBindPipeline(m_primary_command_buffers_[m_current_frame_], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

VkCommandBuffer VulkanRenderer::get_one_time_cmd_buffer() const {
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandBufferCount = 1;
    allocate_info.commandPool = m_command_pool_;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_logical_device_, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void VulkanRenderer::end_one_time_cmd_buffer(VkCommandBuffer cmd_buffer) const {
    vkEndCommandBuffer(cmd_buffer);
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;
    vkQueueSubmit(m_graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue_);
    vkFreeCommandBuffers(m_logical_device_, m_command_pool_, 1, &cmd_buffer);
}

void VulkanRenderer::create_buffer(VkDeviceSize size, VkBufferUsageFlags buffer_flags, VmaAllocationCreateFlags allocation_flags, VkBuffer* buffer, VmaAllocation* allocation) const {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = buffer_flags;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation_create_info{};
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_create_info.flags = allocation_flags;

    VULKAN_ASSERT(vmaCreateBuffer(m_allocator_, &buffer_create_info, &allocation_create_info, buffer, allocation, nullptr), "Failed to create buffer")
}

void VulkanRenderer::copy_buffer(VkBuffer source, VkBuffer dest, VkDeviceSize size) const {
    VkCommandBuffer cmd_buffer = get_one_time_cmd_buffer();
    VkBufferCopy copy_region;
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(cmd_buffer, source, dest, 1, &copy_region);
    end_one_time_cmd_buffer(cmd_buffer);
}

VkExtent2D VulkanRenderer::get_swap_chain_extent() const {
    return m_swap_chain_->get_swap_chain_extent();
}

float VulkanRenderer::get_aspect_ratio() const {
    VkExtent2D extent = m_swap_chain_->get_swap_chain_extent();
    return static_cast<float>(extent.width) / static_cast<float>(extent.height);
}

bool VulkanRenderer::should_close_window() const {
    return m_window_.should_close();
}

void VulkanRenderer::poll() {
    m_window_.glfw_poll_events();
}

GLFWwindow* VulkanRenderer::raw_glfw_window() const {
    return m_window_.raw_window();
}

VkDevice VulkanRenderer::vulkan_device() const {
    return m_logical_device_;
}

VkPhysicalDevice VulkanRenderer::vulkan_physical_device() const {
    return m_physical_device_;
}

VkRenderPass VulkanRenderer::render_pass() const {
    return m_swap_chain_->get_current_render_pass();
}

VkCommandPool VulkanRenderer::command_pool() const {
    return m_command_pool_;
}

VmaAllocator VulkanRenderer::vma_allocator() const {
    return m_allocator_;
}

}