//
// Created by anton on 4/25/21.
//

#include "passIterator.h"
#include <climits>

namespace cyclonite::compositor {
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

auto PassIterator::operator*() const
  -> std::tuple<PassType, VkDescriptorPool, VkDescriptorSetLayout, VkPipelineLayout, VkPipeline, VkDescriptorSet*, bool>
{
    assert(cursor_ < count_);

    auto passType = *(basePassType_ + cursor_);
    auto descriptorPool = static_cast<VkDescriptorPool>(*(baseDescriptorPool_ + cursor_));
    auto descriptorSetLayout = static_cast<VkDescriptorSetLayout>(*(baseDescriptorSetLayout_ + cursor_));
    auto pipelineLayout = static_cast<VkPipelineLayout>(*(basePipelineLayout_ + cursor_));
    auto pipeline = static_cast<VkPipeline>(*(basePipeline_ + cursor_));
    auto descriptorSet = baseDescriptorSet_ + cursor_;

    return std::make_tuple(passType,
                           descriptorPool,
                           descriptorSetLayout,
                           pipelineLayout,
                           pipeline,
                           descriptorSet,
                           bitsWrapper_.isExpired(cursor_));
}
}
