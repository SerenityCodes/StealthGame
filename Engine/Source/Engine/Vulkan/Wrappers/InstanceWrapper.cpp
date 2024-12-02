#include "InstanceWrapper.h"

#include <assert.h>

#include <cstring>
#include <stdexcept>

#include "Engine/Containers/DynArray.h"
#include "GLFW/glfw3.h"

namespace engine::vulkan {

bool check_validation_layer_support(const DynArray<const char*>& needed_layers) {
    
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

    DynArray<VkLayerProperties> available_layers(static_cast<uint16_t>(layer_count));
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    for (auto it = needed_layers.cbegin(); it != needed_layers.cend(); ++it) {
        bool layer_found = false;
        for (const auto& layer_properties : available_layers) {
            if (strcmp(*it, layer_properties.layerName) == 0) {
                layer_found = true;
                break;
            }
        }
        if (!layer_found) {
            return false;
        }
    }
    return true;
}

InstanceWrapper::InstanceWrapper(bool enable_validation_layers) : m_enabled_validation_layers_({"VK_LAYER_KHRONOS_validation"}) {
    if (!enable_validation_layers) {
        m_enabled_validation_layers_ = {};
    }
    if (enable_validation_layers && !check_validation_layer_support(m_enabled_validation_layers_)) {
        assert(false);
    }
    
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "StealthEngine";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "StealthEngine";
    app_info.engineVersion = 1;
    app_info.apiVersion = VK_API_VERSION_1_3;
    app_info.pNext = nullptr;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    
    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.flags = 0;
    create_info.pApplicationInfo = &app_info;

    uint32_t extension_count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extension_count);
    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extensions;

    if (enable_validation_layers) {
        create_info.enabledLayerCount = static_cast<uint32_t>(m_enabled_validation_layers_.get_size());
        create_info.ppEnabledLayerNames = m_enabled_validation_layers_.data();
    } else {
        create_info.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&create_info, nullptr, &m_instance_) != VK_SUCCESS) {
        assert(false);
    }
}

InstanceWrapper::~InstanceWrapper() {
    vkDestroyInstance(m_instance_, nullptr);
}

InstanceWrapper::operator VkInstance() const {
    return m_instance_;
}

const DynArray<const char*>& InstanceWrapper::enabled_validation_layers() const {
    return m_enabled_validation_layers_;
}

}