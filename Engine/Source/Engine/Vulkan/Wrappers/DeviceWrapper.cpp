#include "DeviceWrapper.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include <iostream>
#include <ostream>

#define VMA_IMPLEMENTATION
#include "../Vendor/vma/include/vk_mem_alloc.h"

#include "SwapChain.h"

namespace engine::vulkan {

namespace {

bool is_device_suitable(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader;
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
    throw std::runtime_error("failed to find suitable GPU!");
}

}

DeviceWrapper::DeviceWrapper(Arena& temp_arena, VkSurfaceKHR surface, VkInstance instance, const std::array<const char*, 1>& validation_layers) : m_device_(VK_NULL_HANDLE), m_physical_device_(pick_physical_device(temp_arena, surface, instance)) {
    QueueWrapper::QueueFamily family = QueueWrapper::find_indices(temp_arena, surface, m_physical_device_);
    m_graphics_queue_family_ = family;
    uint32_t indices[] = {family.graphics_family_index, family.present_family_index};
    ArrayRef<VkDeviceQueueCreateInfo> create_infos;
    constexpr float queue_priority = 1.0f;
    if (indices[0] != indices[1]) {
        constexpr int queues_to_create = 2;
        create_infos = ArrayRef{static_cast<VkDeviceQueueCreateInfo*>(temp_arena.push(sizeof(VkDeviceQueueCreateInfo) * queues_to_create)), queues_to_create};
        for (int i = 0; i < 2; i++) {
            create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_infos[i].queueFamilyIndex = indices[i];
            create_infos[i].queueCount = 1;
            create_infos[i].pQueuePriorities = &queue_priority;
            create_infos[i].pNext = nullptr;
        }
    } else {
        constexpr int queues_to_create = 1;
        create_infos = ArrayRef{static_cast<VkDeviceQueueCreateInfo*>(temp_arena.push(sizeof(VkDeviceQueueCreateInfo))), queues_to_create};
        VkDeviceQueueCreateInfo create_info;
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = indices[0];
        create_info.queueCount = 1;
        create_info.pQueuePriorities = &queue_priority;
        create_info.flags = 0;
        create_info.pNext = nullptr;
        create_infos[0] = create_info;
    }

    const char* enabled_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = enabled_extensions;
    create_info.queueCreateInfoCount = static_cast<uint32_t>(create_infos.size());
    create_info.pQueueCreateInfos = create_infos.data();
#ifdef DEBUG
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
#endif
    assert(m_device_ == VK_NULL_HANDLE);
    if (vkCreateDevice(m_physical_device_, &create_info, nullptr, &m_device_) != VK_SUCCESS) {
        assert(false);
    }
    if (create_infos.size() == 1) {
        m_graphics_queue_ = QueueWrapper{m_device_, indices[0]};
        m_present_queue_ = QueueWrapper{m_device_, indices[0]};
    } else {
        m_graphics_queue_ = QueueWrapper{m_device_, indices[0]};
        m_present_queue_ = QueueWrapper{m_device_, indices[1]};
    }

    VmaAllocatorCreateInfo allocator_create_info{};
    allocator_create_info.device = m_device_;
    allocator_create_info.instance = instance;
    allocator_create_info.physicalDevice = m_physical_device_;
    allocator_create_info.vulkanApiVersion = VK_API_VERSION_1_3;
    allocator_create_info.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
    vmaCreateAllocator(&allocator_create_info, &m_allocator_);
}

DeviceWrapper::~DeviceWrapper() {
    vmaDestroyAllocator(m_allocator_);
    vkDestroyDevice(m_device_, nullptr);
}

DeviceWrapper::operator VkDevice() const {
    return m_device_;
}

VkDevice DeviceWrapper::get_logical_device() const {
    return m_device_;
}

VkPhysicalDevice DeviceWrapper::get_physical_device() const {
    return m_physical_device_;
}

QueueWrapper DeviceWrapper::get_graphics_queue() const {
    return m_graphics_queue_;
}

QueueWrapper DeviceWrapper::get_present_queue() const {
    return m_present_queue_;
}

QueueWrapper::QueueFamily DeviceWrapper::get_graphics_queue_family() const {
    return m_graphics_queue_family_;
}

VmaAllocator DeviceWrapper::get_allocator() const {
    return m_allocator_;
}

VkCommandBuffer DeviceWrapper::get_one_time_command_buffer(VkCommandPool command_pool) const {
    VkCommandBufferAllocateInfo allocate_info{};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandBufferCount = 1;
    allocate_info.commandPool = command_pool;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(m_device_, &allocate_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(command_buffer, &begin_info);
    return command_buffer;
}

void DeviceWrapper::end_one_time_command_buffer(VkCommandPool command_pool, VkCommandBuffer cmd_buffer) const {
    vkEndCommandBuffer(cmd_buffer);
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buffer;
    vkQueueSubmit(m_graphics_queue_, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue_);
    vkFreeCommandBuffers(m_device_, command_pool, 1, &cmd_buffer);
}

void DeviceWrapper::copy_buffer(VkCommandPool command_pool, VkBuffer source_buffer, VkBuffer dest_buffer,
    VkDeviceSize size) const {
    VkCommandBuffer cmd_buffer = get_one_time_command_buffer(command_pool);

    VkBufferCopy copy_region;
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = size;
    vkCmdCopyBuffer(cmd_buffer, source_buffer, dest_buffer, 1, &copy_region);
    end_one_time_command_buffer(command_pool, cmd_buffer);
}

uint32_t DeviceWrapper::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device_, &memory_properties);
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) &&
            (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
            }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void DeviceWrapper::create_buffer(VkDeviceSize size, VkBufferUsageFlags buffer_flags, VmaAllocationCreateFlags allocation_flags, VkBuffer* buffer,
    VmaAllocation* allocation) const {
    VkBufferCreateInfo buffer_create_info{};
    buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.size = size;
    buffer_create_info.usage = buffer_flags;
    buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation_create_info{};
    allocation_create_info.usage = VMA_MEMORY_USAGE_AUTO;
    allocation_create_info.flags = allocation_flags;

    if (vmaCreateBuffer(m_allocator_, &buffer_create_info, &allocation_create_info, buffer, allocation, nullptr) != VK_SUCCESS) {
        std::cerr << "Failed to create buffer!" << std::endl;
        assert(false);
    }
}

}