//
// Created by anton on 11/9/25.
//

#ifndef CYCLONITE_VK_INTERNAL_RESOURCE_MANAGER_H
#define CYCLONITE_VK_INTERNAL_RESOURCE_MANAGER_H

#include "core/resourceManager.h"
#include "gfx/vulkan/vkDescriptorSetLayout.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan::internal {
using internal_resource_manager_t = core::ResourceManager<vulkan::DescriptorSetLayout>;
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_INTERNAL_RESOURCE_MANAGER_H
