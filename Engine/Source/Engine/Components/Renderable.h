#pragma once
#include <memory>

#include "Engine/Vulkan/VulkanModel.h"

namespace components {

struct Renderable {
    engine::vulkan::VulkanModel* model;
};

}
