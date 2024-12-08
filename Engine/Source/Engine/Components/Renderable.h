#pragma once

#include "Engine/Vulkan/VulkanModel.h"

namespace components {

struct Renderable {
    engine::vulkan::VulkanModel* model;
};

}
