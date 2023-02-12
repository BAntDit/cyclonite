//
// Created by bantdit on 1/15/23.
//

#include "descriptorSetManager.h"
#include "device.h"
#include "internal/descriptorSetHelpers.h"
#include "render/descriptorType.h"

namespace cyclonite::vulkan {
using namespace easy_mp;

DescriptorSetManager::DescriptorSetManager(Device& device, multithreading::TaskManager& taskManager)
  : device_{ &device }
  , taskManager_{ &taskManager }
{}

auto DescriptorSetManager::_addPool(
  DescriptorPoolKey poolKey,
  uint32_t capacity,
  std::vector<std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t, size_t>>& pools)
  -> std::tuple<vulkan::SharedHandle<VkDescriptorPool> const&, size_t&>
{
    assert(multithreading::Render::isInRenderThread());

    auto poolCreateInfo = VkDescriptorPoolCreateInfo{};

    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolCreateInfo.maxSets = capacity;

    auto poolSizeCount = uint32_t{ 0 };
    auto poolSizes = std::array<VkDescriptorPoolSize, 16>{};

    auto descriptorTypes = { DescriptorType::SAMPLER,
                             DescriptorType::COMBINED_IMAGE_SAMPLER,
                             DescriptorType::SAMPLED_IMAGE,
                             DescriptorType::STORAGE_IMAGE,
                             DescriptorType::UNIFORM_TEXEL_BUFFER,
                             DescriptorType::STORAGE_TEXEL_BUFFER,
                             DescriptorType::UNIFORM_BUFFER,
                             DescriptorType::STORAGE_BUFFER,
                             DescriptorType::UNIFORM_BUFFER_DYNAMIC,
                             DescriptorType::STORAGE_BUFFER_DYNAMIC,
                             DescriptorType::INPUT_ATTACHMENT,
                             DescriptorType::INLINE_UNIFORM_BLOCK,
                             DescriptorType::ACCELERATION_STRUCTURE,
                             DescriptorType::MUTABLE_VALUE };

    for (auto descriptorType : descriptorTypes) {
        auto count = poolKey.get(descriptorType);

        if (count > 0) {
            auto& poolSize = poolSizes[poolSizeCount++];

            poolSize.type = internal::descriptorTypeToVulkanDescriptorType(descriptorType);
            poolSize.descriptorCount = count * capacity;
        }
    }

    poolCreateInfo.poolSizeCount = poolSizeCount;
    poolCreateInfo.pPoolSizes = poolSizes.data();

    auto&& [pool, _, usage] = pools.emplace_back(
      vulkan::SharedHandle<VkDescriptorPool>{ device_->handle(), vkDestroyDescriptorPool }, capacity, 0);
    (void)_;

    if (auto result = vkCreateDescriptorPool(device_->handle(), &poolCreateInfo, nullptr, &pool);
        result != VK_SUCCESS) {
        throw std::runtime_error("can not create descriptor pool");
    }

    return std::tie(pool, usage);
}

auto DescriptorSetManager::addPool(DescriptorPoolKey poolKey, uint32_t capacity)
  -> std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t&>
{
    auto&& addPoolTask = [this, poolKey, capacity]() -> std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t&> {
        auto pool = vulkan::SharedHandle<VkDescriptorPool>{};
        auto usagePtr = std::add_pointer_t<size_t>{ nullptr };

        auto it = pools_.find(static_cast<packed_descriptor_set_key_t>(poolKey));
        if (it == pools_.end()) {
            auto [pit, success] = pools_.emplace(
              poolKey, std::vector<std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t, size_t>>{});
            assert(success);
            (void)success;

            auto [key, pools] = *pit;
            (void)key;

            auto&& [p, u] = _addPool(poolKey, capacity, pools);
            pool = p;
            usagePtr = &u;
        }

        return std::tie(pool, *usagePtr);
    };

    return multithreading::Render::isInRenderThread() ? addPoolTask()
                                                      : taskManager_->submitRenderTask(std::move(addPoolTask)).get();
}

auto DescriptorSetManager::_allocDescriptorSet(DescriptorPoolKey poolKey,
                                               VkDescriptorSetLayout const* layouts,
                                               uint32_t count,
                                               uint32_t newPoolCapacity) -> PipelineDescriptorSets
{
    assert(count <= maxDescriptorSetsPerPipeline);

    auto&& allocTask = [this, poolKey, count, layouts, newPoolCapacity]() -> PipelineDescriptorSets {
        auto descriptorSets = PipelineDescriptorSets{};

        if (!pools_.contains(static_cast<packed_descriptor_set_key_t>(poolKey))) {
            addPool(poolKey, newPoolCapacity);
        }
        assert(pools_.contains(static_cast<packed_descriptor_set_key_t>(poolKey)));

        auto&& pools = pools_.at(static_cast<packed_descriptor_set_key_t>(poolKey));

        uint32_t lastCapacity = 0;

        for (auto& [pool, capacity, usage] : pools) {
            lastCapacity = capacity;

            if (usage < capacity) {
                auto descriptorSetAllocateInfo = VkDescriptorSetAllocateInfo{};

                descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                descriptorSetAllocateInfo.descriptorPool = static_cast<VkDescriptorPool>(pool);
                descriptorSetAllocateInfo.descriptorSetCount = count;
                descriptorSetAllocateInfo.pSetLayouts = layouts;

                auto result =
                  vkAllocateDescriptorSets(device_->handle(), &descriptorSetAllocateInfo, descriptorSets.sets_.data());

                if (result == VK_SUCCESS) {
                    descriptorSets.manager_ = this;
                    descriptorSets.pool_ = pool;
                    descriptorSets.key_ = static_cast<packed_descriptor_set_key_t>(poolKey);
                    descriptorSets.count_ = count;
                    descriptorSets.needsRewriteFlags_.reset();

                    usage++;

                    break;
                } else if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
                    continue;
                } else {
                    throw std::runtime_error("could not allocate descriptor sets");
                }
            }
        }

        if (descriptorSets.count_ == 0) {
            auto&& [pool, usage] = _addPool(poolKey, lastCapacity * 2, pools);

            auto descriptorSetAllocateInfo = VkDescriptorSetAllocateInfo{};

            descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            descriptorSetAllocateInfo.descriptorPool = static_cast<VkDescriptorPool>(pool);
            descriptorSetAllocateInfo.descriptorSetCount = count;
            descriptorSetAllocateInfo.pSetLayouts = layouts;

            auto result =
              vkAllocateDescriptorSets(device_->handle(), &descriptorSetAllocateInfo, descriptorSets.sets_.data());

            if (result == VK_SUCCESS) {
                descriptorSets.manager_ = this;
                descriptorSets.pool_ = pool;
                descriptorSets.key_ = static_cast<packed_descriptor_set_key_t>(poolKey);
                descriptorSets.count_ = count;
                descriptorSets.needsRewriteFlags_.reset();

                usage++;
            } else {
                // it is not possible:
                assert(result != VK_ERROR_OUT_OF_POOL_MEMORY && result != VK_ERROR_FRAGMENTED_POOL);
                throw std::runtime_error("could not allocate descriptor sets");
            }
        }

        assert(descriptorSets.count_ > 0);

        return descriptorSets;
    };

    return multithreading::Render::isInRenderThread() ? allocTask()
                                                      : taskManager_->submitRenderTask(std::move(allocTask)).get();
}

void DescriptorSetManager::freeDescriptorSet(PipelineDescriptorSets& descriptorSets)
{
    assert(descriptorSets.count_ > 0);

    auto&& freeTask = [this, &descriptorSets]() -> void {
        assert(pools_.contains(descriptorSets.key_));

        auto& pools = pools_.at(descriptorSets.key_);

        for (auto&& [pool, capacity, usage] : pools) {
            (void)capacity;

            if (static_cast<VkDescriptorPool>(pool) == static_cast<VkDescriptorPool>(descriptorSets.pool_)) {
                assert(usage > 0);

                if (auto result = vkFreeDescriptorSets(device_->handle(),
                                                       static_cast<VkDescriptorPool>(pool),
                                                       descriptorSets.count_,
                                                       descriptorSets.sets_.data());
                    result == VK_SUCCESS) {
                    usage--;
                } else {
                    throw std::runtime_error("could not free descriptor sets");
                }
            }
        }
    };

    return multithreading::Render::isInRenderThread() ? freeTask()
                                                      : taskManager_->submitRenderTask(std::move(freeTask)).get();
}
}
