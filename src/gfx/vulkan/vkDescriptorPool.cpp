
#include "vkDescriptorPool.h"
#include "gfx/device.h"
#include "vkDescriptorSetLayout.h"
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
  , deviceRef_{ std::move(deviceRef) }
  , layoutRef_{ std::move(layoutRef) }
  , vkDescriptorPool_{ deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>().handle(),
                       vkDestroyDescriptorPool }
  , maxSets_{ maxSets }
  , flags_{}
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

    assert(layoutRef_.valid());
    auto& layout = layoutRef_.as<DescriptorSetLayout>();

    if (allowIndividualReset)
        flags_ |= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    if ((layout.flags() & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0)
        flags_ |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

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
        poolSizeses.back().descriptorCount += b.descriptorCount * maxSets_;
    }

    auto poolCreateInfo = VkDescriptorPoolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.flags = flags_;
    poolCreateInfo.maxSets = maxSets_;
    poolCreateInfo.poolSizeCount = poolSizeses.size();
    poolCreateInfo.pPoolSizes = poolSizeses.data();

    if (auto vkResult = vkCreateDescriptorPool(device.handle(), &poolCreateInfo, nullptr, &vkDescriptorPool_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateDescriptorPool" };
    }
}

auto DescriptorPool::allocateDescriptorSet() -> VkDescriptorSet
{
    auto allocatedSet = VkDescriptorSet{ VK_NULL_HANDLE };

    auto allocationCount = allocationCount_.load(std::memory_order_relaxed);

    while (allocationCount < maxSets_ &&
           !allocationCount_.compare_exchange_weak(
             allocationCount, allocationCount + 1, std::memory_order_acq_rel, std::memory_order_acquire)) {
    }

    if (allocationCount < maxSets_) {
        assert(deviceRef_.valid());
        auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

        assert(layoutRef_.valid());
        auto& layout = layoutRef_.as<DescriptorSetLayout>();
        auto vkLayout = layout.handle();

        auto allocationInfo = VkDescriptorSetAllocateInfo{};
        allocationInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocationInfo.descriptorPool = static_cast<VkDescriptorPool>(vkDescriptorPool_);
        allocationInfo.descriptorSetCount = 1;
        allocationInfo.pSetLayouts = &vkLayout;

        if (auto vkResult = vkAllocateDescriptorSets(device.handle(), &allocationInfo, &allocatedSet);
            vkResult != VK_SUCCESS) {
            throw Exception{ vkResult, "vkAllocateDescriptorSets" };
        }
    }

    return allocatedSet;
}

void DescriptorPool::freeDescriptorSet(VkDescriptorSet vkDescriptorSet)
{
    if ((flags_ & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0) {
        auto allocationCount = allocationCount_.load(std::memory_order_relaxed);

        while (allocationCount > 0 &&
               !allocationCount_.compare_exchange_weak(
                 allocationCount, allocationCount - 1, std::memory_order_acq_rel, std::memory_order_acquire)) {
        }

        if (allocationCount > 0) {
            assert(deviceRef_.valid());
            auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

            if (auto vkResult = vkFreeDescriptorSets(
                  device.handle(), static_cast<VkDescriptorPool>(vkDescriptorPool_), 1, &vkDescriptorSet);
                vkResult != VK_SUCCESS) {
                throw Exception{ vkResult, "vkFreeDescriptorSets" };
            }
        }
    } else {
        deallocationCount_.fetch_add(1, std::memory_order_release);
    }
}

auto DescriptorPool::unused() const -> bool
{
    return ((flags_ & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT) != 0)
             ? allocationCount_.load(std::memory_order_acquire) == 0
             : deallocationCount_.load(std::memory_order_acquire) == maxSets_;
}
}
#endif
