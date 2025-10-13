//
// Created by anton on 9/7/25.
//

#include "vkCommandPool.h"
#include "gfx/resourceManager.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
CommandPool::CommandPool(core::ResourceManagerBase* resourceManager,
                         core::ResourceId resourceId,
                         core::ResourceSharedRef deviceRef,
                         uint32_t queueFamilyIndex,
                         CommandPoolFlagBits flags)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , core::EnableRefFromThis{}
  , deviceRef_{ deviceRef }
  , vkCommandPool_{ deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>().handle(), vkDestroyCommandPool }
  , threadId_{ std::this_thread::get_id() }
  , queueFamilyIndex_{ queueFamilyIndex }
  , flags_{ flags }
{
    assert(deviceRef_.valid());

    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

    auto createInfo = VkCommandPoolCreateInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilyIndex;
    createInfo.flags = flags_.cast_to<VkCommandPoolCreateFlags>();

    if (auto vkResult = vkCreateCommandPool(device.handle(), &createInfo, nullptr, &vkCommandPool_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateCommandPool" };
    }
}

auto CommandPool::allocCommandList() -> gfx::CommandList
{
    auto weakRef = getWeakFromThis(this);
    auto commandList = gfx::CommandList{ weakRef };
    return commandList;
}

void CommandPool::reset(bool releasePoolResources /* = false*/)
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

    auto flags = releasePoolResources ? VkCommandPoolResetFlags{ VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT }
                                      : VkCommandPoolResetFlags{};

    if (auto vkResult = vkResetCommandPool(device.handle(), static_cast<VkCommandPool>(vkCommandPool_), flags);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkResetCommandPool" };
    }
}

#endif
