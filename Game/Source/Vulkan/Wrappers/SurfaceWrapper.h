#pragma once
#include <vulkan/vulkan_core.h>

#include "Window.h"

namespace engine::vulkan {

class SurfaceWrapper {
    VkInstance m_instance_;
    VkSurfaceKHR m_surface_;
public:
    SurfaceWrapper(VkInstance instance, GLFWwindow* window);
    ~SurfaceWrapper();

    operator VkSurfaceKHR() const;
};

}
