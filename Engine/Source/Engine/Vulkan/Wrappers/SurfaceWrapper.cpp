#include "SurfaceWrapper.h"

#include <cassert>

namespace engine::vulkan {

SurfaceWrapper::SurfaceWrapper(VkInstance instance,
    GLFWwindow* window) : m_instance_(instance) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &m_surface_) != VK_SUCCESS) {
        assert(false);
    }
}

SurfaceWrapper::~SurfaceWrapper() {
    vkDestroySurfaceKHR(m_instance_, m_surface_, nullptr);
}

SurfaceWrapper::operator VkSurfaceKHR() const {
    return m_surface_;
}

}