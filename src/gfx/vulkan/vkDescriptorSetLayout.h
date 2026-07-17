//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_DESCRIPTOR_SET_LAYOUT_H
#define CYCLONITE_VK_DESCRIPTOR_SET_LAYOUT_H

#include "core/resourceBase.h"
#include "handle.h"
#include <span>
#include <vector>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class DescriptorSetLayout : public core::ResourceBase // internal vk resource
{
public:
    DescriptorSetLayout(core::ResourceManagerBase* resourceManager,
                        core::ResourceId resourceId,
                        VkDevice vkDeviceHandle,
                        VkDescriptorSetLayoutCreateFlags flags,
                        std::span<VkDescriptorBindingFlags const> bindingFlags,
                        std::span<VkDescriptorSetLayoutBinding const> bindings);

    [[nodiscard]] auto handle() const -> VkDescriptorSetLayout
    {
        return static_cast<VkDescriptorSetLayout>(vkDescriptorSetLayout_);
    }

    [[nodiscard]] auto flags() const -> VkDescriptorSetLayoutCreateFlags { return flags_; }

    [[nodiscard]] auto bindings() const -> std::vector<VkDescriptorSetLayoutBinding> const& { return bindings_; }

    using core::ResourceBase::resourceBase;

private:
    std::vector<VkDescriptorSetLayoutBinding> bindings_;
    Handle<VkDescriptorSetLayout> vkDescriptorSetLayout_;
    VkDescriptorSetLayoutCreateFlags flags_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_DESCRIPTOR_SET_LAYOUT_H
