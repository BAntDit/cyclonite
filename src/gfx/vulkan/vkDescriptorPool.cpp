
#include "gfx/device.h"
#include "vkDescriptorSetLayout.h"
#include "vkDescriptorPool.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
DescriptorPool::DescriptorPool(core::ResourceManagerBase* resourceManager,
                               core::ResourceId resourceId,
                               core::ResourceSharedRef deviceRef,
                               core::ResourceSharedRef layoutRef,
                               uint32_t maxSets,
                               bool allowIndividualReset)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , layoutRef_{ std::move(layoutRef) }
  , vkDescriptorPool_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                       vkDestroyDescriptorPool }
{   
    assert(deviceRef.valid());
    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    assert(layoutRef_.valid());
    auto& layout = layoutRef_.as<DescriptorSetLayout>();

    auto flags = VkDescriptorPoolCreateFlags{};
    if (allowIndividualReset)
        flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if ((layout.flags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0)
        flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

    auto const& bindings = layout.bindings();
    auto poolSizeses = std::vector<VkDescriptorPoolSize>{};
    poolSizeses.reserve(bindings.size());

    auto prevDescriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    for (auto const& b : bindings) {
        if (prevDescriptorType != b.descriptorType) {
            auto poolSize = VkDescriptorPoolSize{};
            poolSize.type = b.descriptorType;
            prevDescriptorType = b.descriptorType;
            poolSizeses.emplace_back(poolSize);
        }
        poolSizeses.back().descriptorCount += b.descriptorCount * maxSets;
    }

    auto poolCreateInfo = VkDescriptorPoolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.flags = flags;
    poolCreateInfo.maxSets = maxSets;
    poolCreateInfo.poolSizeCount = poolSizeses.size();
    poolCreateInfo.pPoolSizes = poolSizeses.data();

    if (auto vkResult = vkCreateDescriptorPool(device.handle(), &poolCreateInfo, nullptr, &vkDescriptorPool_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateDescriptorPool" };
    }
}
}
#endif
