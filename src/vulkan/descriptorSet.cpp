//
// Created by bantdit on 1/16/23.
//

#include "descriptorSet.h"

namespace cyclonite::vulkan {
DescriptorSet::DescriptorSet(DescriptorSetManager& manager,
                             vulkan::SharedHandle<VkDescriptorPool> const& pool,
                             VkDescriptorSet set,
                             cyclonite::vulkan::packed_descriptor_set_key_t key)
  : manager_{ &manager }
  , pool_{ pool }
  , set_{ set }
  , key_(key)
{}

DescriptorSet::~DescriptorSet()
{
    // manager->free();
}
}