//
// Created by bantdit on 1/15/23.
//

#ifndef CYCLONITE_PIPELINE_DESCRIPTORSETS_H
#define CYCLONITE_PIPELINE_DESCRIPTORSETS_H

#include "config.h"
#include "descriptorSetKey.h"
#include "sharedHandle.h"
#include <bitset>

namespace cyclonite::vulkan {
class DescriptorSetManager;

class PipelineDescriptorSets
{
    friend class DescriptorSetManager;

private:
    PipelineDescriptorSets() = default;

public:
    PipelineDescriptorSets(PipelineDescriptorSets const&) = delete;

    PipelineDescriptorSets(PipelineDescriptorSets&& pipelineDescriptorSets) noexcept;

    auto operator=(PipelineDescriptorSets const&) -> PipelineDescriptorSets& = delete;

    auto operator=(PipelineDescriptorSets&&) noexcept -> PipelineDescriptorSets&;

    [[nodiscard]] auto descriptorSet(size_t setIndex) const -> VkDescriptorSet;

    [[nodiscard]] auto allSetsPtr() const -> VkDescriptorSet const*;

    [[nodiscard]] auto descriptorSetCount() const -> uint32_t { return count_; }

    [[nodiscard]] auto key() const -> packed_descriptor_set_key_t { return key_; }

    [[nodiscard]] auto needsRewrite() const -> bool;

    [[nodiscard]] auto needsRewrite(size_t setIndex) const -> bool;

    void makeExpired(size_t setIndex);

    void makeExpired();

    ~PipelineDescriptorSets();

private:
    DescriptorSetManager* manager_;
    vulkan::SharedHandle<VkDescriptorPool> pool_;
    packed_descriptor_set_key_t key_;
    std::array<VkDescriptorSet, maxDescriptorSetsPerPipeline> sets_;
    uint32_t count_;
    std::bitset<maxDescriptorSetsPerPipeline> needsRewriteFlags_;
};
}

#endif // CYCLONITE_PIPELINE_DESCRIPTORSETS_H
