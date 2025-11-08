//
// Created by anton on 11/8/25.
//

#ifndef CYCLONITE_VK_DESCRIPTOR_SET_LAYOUT_H
#define CYCLONITE_VK_DESCRIPTOR_SET_LAYOUT_H

#include "core/resourceBase.h"
#include "handle.h"
#include <span>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class DescriptorSetLayout : public core::ResourceBase // internal vk resource
{
public:
    DescriptorSetLayout(core::ResourceManagerBase* resourceManager,
                        core::ResourceId resourceId,
                        VkDevice vkDeviceHandle,
                        VkDescriptorSetLayoutCreateFlags flags,
                        std::span<const VkDescriptorSetLayoutBinding> bindings);

    [[nodiscard]] auto handle() const -> VkDescriptorSetLayout
    {
        return static_cast<VkDescriptorSetLayout>(vkDescriptorSetLayout_);
    }

    using core::ResourceBase::resourceBase;

private:
    Handle<VkDescriptorSetLayout> vkDescriptorSetLayout_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_DESCRIPTOR_SET_LAYOUT_H