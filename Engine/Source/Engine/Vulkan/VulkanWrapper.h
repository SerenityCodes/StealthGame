﻿#pragma once

#include <memory>

#include "Wrappers/CommandBufferWrapper.h"
#include "Wrappers/DeviceWrapper.h"
#include "Wrappers/InstanceWrapper.h"
#include "Wrappers/PipelineWrapper.h"
#include "Wrappers/SurfaceWrapper.h"
#include "Wrappers/Window.h"

namespace engine::vulkan {

class VulkanWrapper {
    Window window_;
    DynArray<const char*> m_validation_layers_;
    InstanceWrapper m_instance_;
    SurfaceWrapper m_surface_;
    DeviceWrapper m_device_;
    std::unique_ptr<SwapChain> m_swap_chain_;
    PipelineWrapper m_pipeline_;
    CommandBufferWrapper m_command_buffer_;

    uint32_t m_current_frame_ = 0;
public:
    VulkanWrapper();
    VulkanWrapper(const VulkanWrapper&) = delete;
    VulkanWrapper(VulkanWrapper&&) = delete;
    VulkanWrapper& operator=(const VulkanWrapper&) = delete;
    VulkanWrapper& operator=(VulkanWrapper&&) = delete;
    ~VulkanWrapper() = default;

    Window& window();

    void draw_frame();
    void reset_swap_chain();
};

}
