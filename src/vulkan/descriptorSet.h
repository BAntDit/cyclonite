//
// Created by bantdit on 1/15/23.
//

#ifndef CYCLONITE_DESCRIPTORSET_H
#define CYCLONITE_DESCRIPTORSET_H

#include "descriptorSetKey.h"
#include "sharedHandle.h"

namespace cyclonite::vulkan {
class DescriptorSetManager;

class DescriptorSet
{
    friend class DescriptorSetManager;

private:
    DescriptorSet(DescriptorSetManager& manager,
                  vulkan::SharedHandle<VkDescriptorPool> const& pool,
                  VkDescriptorSet set,
                  packed_descriptor_set_key_t key);

public:
    DescriptorSet(DescriptorSet const&) = delete;

    DescriptorSet(DescriptorSet&&) = default;

    auto operator=(DescriptorSet const&) -> DescriptorSet& = delete;

    auto operator=(DescriptorSet &&) -> DescriptorSet& = default;

    [[nodiscard]] auto handle() const -> VkDescriptorSet { return set_; }

    [[nodiscard]] auto key() const -> packed_descriptor_set_key_t { return key_; }

    ~DescriptorSet();

private:
    DescriptorSetManager* manager_;
    vulkan::SharedHandle<VkDescriptorPool> pool_;
    VkDescriptorSet set_;
    packed_descriptor_set_key_t key_;
};
}

#endif // CYCLONITE_DESCRIPTORSET_H
