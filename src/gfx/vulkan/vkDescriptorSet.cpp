//
// Created by anton on 11/15/25.
//

#include "vkDescriptorSet.h"
#include "vkDescriptorPool.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
DescriptorSet::DescriptorSet(core::ResourceManagerBase* resourceManager,
                             core::ResourceId resourceId,
                             core::ResourceSharedRef descriptorPool)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , descriptorPool_{ std::move(descriptorPool) }
  , vkDescriptorSet_{ VK_NULL_HANDLE }
{
    assert(descriptorPool_.valid());
    auto& pool = descriptorPool_.as<DescriptorPool>();

    vkDescriptorSet_ = pool.allocateDescriptorSet();
}

DescriptorSet::~DescriptorSet()
{
    assert(descriptorPool_.valid());
    auto& pool = descriptorPool_.as<DescriptorPool>();

    if (vkDescriptorSet_ != VK_NULL_HANDLE) {
        pool.freeDescriptorSet(vkDescriptorSet_);
        vkDescriptorSet_ = VK_NULL_HANDLE;
    }
}
}
#endif