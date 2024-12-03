#include "DeviceWrapper.h"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

#include "SwapChain.h"
#include "Engine/Containers/DynArray.h"

namespace engine::vulkan {

bool is_device_suitable(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    return device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
           device_features.geometryShader;
}

bool check_device_extension_support(VkPhysicalDevice device) {
    const char* needed_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);

    DynArray<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

    auto check_func = [needed_extensions](VkExtensionProperties extension) {
        return strcmp(extension.extensionName, needed_extensions[0]) == 0;
    };
    return std::any_of(available_extensions.begin(), available_extensions.end(), check_func);
}

bool is_swap_chain_good(VkSurfaceKHR surface, VkPhysicalDevice device) {
    bool swap_chain_good = false;
    SwapChain::SupportDetails swap_chain_support_details = SwapChain::create_support_details(surface, device);
    swap_chain_good = !swap_chain_support_details.formats.is_empty() && !swap_chain_support_details.present_modes.is_empty();
    return swap_chain_good;
}

VkPhysicalDevice pick_physical_device(VkSurfaceKHR surface, VkInstance instance) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
    if (device_count == 0) {
        throw std::runtime_error("No suitable GPUs available");
    }
    DynArray<VkPhysicalDevice> physical_devices(static_cast<uint16_t>(device_count));
    vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());
    for (const auto& device : physical_devices) {
        
        if (is_device_suitable(device) && check_device_extension_support(device) && is_swap_chain_good(surface, device)) {
            return device;
        }
    }
    // Not supposed to make it here
    throw std::runtime_error("failed to find suitable GPU!");
}

DeviceWrapper::DeviceWrapper(VkSurfaceKHR surface, VkInstance instance, const DynArray<const char*>& validation_layers) : m_device_(VK_NULL_HANDLE), m_physical_device_(pick_physical_device(surface, instance)) {
    QueueWrapper::QueueFamily family = QueueWrapper::find_indices(surface, m_physical_device_);
    m_graphics_queue_family_ = family;
    uint32_t indices[] = {family.graphics_family_index, family.present_family_index};
    VkDeviceQueueCreateInfo* create_infos;
    VkDeviceQueueCreateInfo stack_create_infos[2];
    int queue_create_count;
    float queue_priority = 1.0f;
    if (indices[0] != indices[1]) {
        queue_create_count = 2;
        create_infos = stack_create_infos;
        for (int i = 0; i < 2; i++) {
            create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_infos[i].queueFamilyIndex = indices[i];
            create_infos[i].queueCount = 1;
            create_infos->pQueuePriorities = &queue_priority;
            create_infos[i].pNext = nullptr;
        }
    } else {
        queue_create_count = 1;
        VkDeviceQueueCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        create_info.queueFamilyIndex = indices[0];
        create_info.queueCount = queue_create_count;
        create_info.pQueuePriorities = &queue_priority;
        create_info.flags = 0;
        create_info.pNext = nullptr;
        create_infos = &create_info;
    }

    const char* enabled_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    
    VkDeviceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.enabledExtensionCount = 1;
    create_info.ppEnabledExtensionNames = enabled_extensions;
    create_info.queueCreateInfoCount = queue_create_count;
    create_info.pQueueCreateInfos = create_infos;
    create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.get_size());
    if (validation_layers.get_size() != 0) {
        create_info.ppEnabledLayerNames = validation_layers.data();
    }
    if (vkCreateDevice(m_physical_device_, &create_info, nullptr, &m_device_) != VK_SUCCESS) {
        assert(false);
    }

    if (queue_create_count == 1) {
        m_graphics_queue_ = QueueWrapper{m_device_, indices[0]};
        m_present_queue_ = QueueWrapper{m_device_, indices[0]};
    } else {
        m_graphics_queue_ = QueueWrapper{m_device_, indices[0]};
        m_present_queue_ = QueueWrapper{m_device_, indices[1]};
    }
}

DeviceWrapper::~DeviceWrapper() {
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

}