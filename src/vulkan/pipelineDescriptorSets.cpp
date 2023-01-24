//
// Created by bantdit on 1/16/23.
//

#include "pipelineDescriptorSets.h"
#include "descriptorSetManager.h"

namespace cyclonite::vulkan {
PipelineDescriptorSets::PipelineDescriptorSets(PipelineDescriptorSets&& pipelineDescriptorSets) noexcept
  : manager_{ pipelineDescriptorSets.manager_ }
  , pool_{ std::move(pipelineDescriptorSets.pool_) }
  , key_(pipelineDescriptorSets.key_)
  , sets_(pipelineDescriptorSets.sets_)
  , count_{ pipelineDescriptorSets.count_ }
  , needsRewriteFlags_(pipelineDescriptorSets.needsRewriteFlags_)
{
    pipelineDescriptorSets.manager_ = nullptr;
    pipelineDescriptorSets.key_ = packed_descriptor_set_key_t{};

    std::fill(
      pipelineDescriptorSets.sets_.begin(), pipelineDescriptorSets.sets_.end(), VkDescriptorSet{ VK_NULL_HANDLE });

    pipelineDescriptorSets.count_ = 0;
    pipelineDescriptorSets.needsRewriteFlags_.reset();
}

auto PipelineDescriptorSets::operator=(PipelineDescriptorSets&& rhs) noexcept -> PipelineDescriptorSets&
{
    manager_ = rhs.manager_;
    rhs.manager_ = nullptr;

    pool_ = std::move(rhs.pool_);

    key_ = rhs.key_;
    rhs.key_ = packed_descriptor_set_key_t{};

    sets_ = rhs.sets_;
    std::fill(rhs.sets_.begin(), rhs.sets_.end(), VkDescriptorSet{ VK_NULL_HANDLE });

    count_ = rhs.count_;
    rhs.count_ = 0;

    needsRewriteFlags_ = rhs.needsRewriteFlags_;
    rhs.needsRewriteFlags_.reset();

    return *this;
}

auto PipelineDescriptorSets::descriptorSet(size_t setIndex) const -> VkDescriptorSet
{
    assert(setIndex < sets_.size());
    return sets_[setIndex];
}

auto PipelineDescriptorSets::allSetsPtr() const -> VkDescriptorSet const*
{
    return sets_.data();
}

auto PipelineDescriptorSets::needsRewrite() const -> bool
{
    return needsRewriteFlags_.any();
}

auto PipelineDescriptorSets::needsRewrite(size_t setIndex) const -> bool
{
    return needsRewriteFlags_.test(setIndex);
}

auto PipelineDescriptorSets::makeExpired(size_t setIndex)
{
    needsRewriteFlags_.reset(setIndex);
}

auto PipelineDescriptorSets::makeExpired()
{
    needsRewriteFlags_.reset();
}

PipelineDescriptorSets::~PipelineDescriptorSets()
{
    if (manager_ != nullptr) {
        // TODO:: manager_->freeDescriptorSet(pool_);
    }
}
}