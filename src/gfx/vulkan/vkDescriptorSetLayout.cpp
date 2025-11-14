//
// Created by anton on 11/8/25.
//

#include "vkDescriptorSetLayout.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
DescriptorSetLayout::DescriptorSetLayout(core::ResourceManagerBase* resourceManager,
                                         core::ResourceId resourceId,
                                         VkDevice vkDeviceHandle,
                                         VkDescriptorSetLayoutCreateFlags flags,
                                         std::span<VkDescriptorBindingFlags const> bindingFlags,
                                         std::span<VkDescriptorSetLayoutBinding const> bindings)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , bindings_(bindings.begin(), bindings.end())
  , vkDescriptorSetLayout_{ vkDeviceHandle, vkDestroyDescriptorSetLayout }
  , flags_{ flags }
{
    auto bindingFlagsCreateInfo = VkDescriptorSetLayoutBindingFlagsCreateInfo{};
    bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
    bindingFlagsCreateInfo.bindingCount = bindingFlags.size();
    bindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

    auto createInfo = VkDescriptorSetLayoutCreateInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = &bindingFlagsCreateInfo;
    createInfo.flags = flags;
    createInfo.bindingCount = bindings_.size();
    createInfo.pBindings = bindings_.data();

    if (auto vkResult = vkCreateDescriptorSetLayout(vkDeviceHandle, &createInfo, nullptr, &vkDescriptorSetLayout_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateDescriptorSetLayout" };
    }
}
}
#endif