//
// Created by anton on 4/25/21.
//

#include "passIterator.h"
#include <climits>

namespace cyclonite::compositor {
PassIterator::PassIterator(uint32_t passCount,
                           uint32_t cursor,
                           PassType* passType,
                           vulkan::Handle<VkDescriptorPool>* baseDescriptorPool,
                           vulkan::Handle<VkDescriptorSetLayout>* baseDescriptorSetLayout,
                           vulkan::Handle<VkPipelineLayout>* basePipelineLayout,
                           vulkan::Handle<VkPipeline>* basePipeline,
                           VkDescriptorSet* baseDescriptorSet,
                           std::byte* baseExpirationBitsByte) noexcept
  : count_{ passCount }
  , cursor_{ cursor }
  , basePassType_{ passType }
  , baseDescriptorPool_{ baseDescriptorPool }
  , baseDescriptorSetLayout_{ baseDescriptorSetLayout }
  , basePipelineLayout_{ basePipelineLayout }
  , basePipeline_{ basePipeline }
  , baseDescriptorSet_{ baseDescriptorSet }
  , baseExpirationBitsByte_{ baseExpirationBitsByte }
{}

auto PassIterator::operator++() -> PassIterator&
{
    assert(cursor_ < count_);
    cursor_++;
    return *this;
}

auto PassIterator::operator++(int) -> PassIterator
{
    auto tmp = *this;

    ++*this;

    return tmp;
}

auto PassIterator::operator*() const -> std::
  tuple<PassType, VkDescriptorPool, VkDescriptorSetLayout, VkPipelineLayout, VkPipeline, VkDescriptorSet*, std::byte*>
{
    assert(cursor_ < count_);

    auto passType = *(basePassType_ + cursor_);
    auto descriptorPool = static_cast<VkDescriptorPool>(*(baseDescriptorPool_ + cursor_));
    auto descriptorSetLayout = static_cast<VkDescriptorSetLayout>(*(baseDescriptorSetLayout_ + cursor_));
    auto pipelineLayout = static_cast<VkPipelineLayout>(*(basePipelineLayout_ + cursor_));
    auto pipeline = static_cast<VkPipeline>(*(basePipeline_ + cursor_));
    auto descriptorSet = baseDescriptorSet_ + cursor_;
    auto flags = baseExpirationBitsByte_ + (cursor_ / CHAR_BIT);

    return std::make_tuple(
      passType, descriptorPool, descriptorSetLayout, pipelineLayout, pipeline, descriptorSet, flags);
}
}
