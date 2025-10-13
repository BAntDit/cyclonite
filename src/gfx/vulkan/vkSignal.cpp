//
// Created by anton on 9/27/25.
//

#include "vkSignal.h"
#include "gfx/device.h"
#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)

namespace cyclonite::gfx::vulkan {
Signal::Signal(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               core::ResourceSharedRef deviceRef,
               SignalType signalType,
               uint64_t initialValue /* = 0*/)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , deviceRef_{ deviceRef }
  , vkSemaphore_{ deviceRef.as<gfx::type_traits::platform_implementation_t<gfx::Device>>().handle(),
                  vkDestroySemaphore }
  , type_{ signalType }
{
    assert(deviceRef.valid());
    auto& device = deviceRef.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    auto semaphoreTypeCreateInfo = VkSemaphoreTypeCreateInfo{};
    semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
    semaphoreTypeCreateInfo.semaphoreType =
      signalType == SignalType::BINARY ? VK_SEMAPHORE_TYPE_BINARY : VK_SEMAPHORE_TYPE_TIMELINE;
    semaphoreTypeCreateInfo.initialValue = initialValue;

    auto semaphoreCreateInfo = VkSemaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = &semaphoreTypeCreateInfo;

    if (auto vkResult = vkCreateSemaphore(device.handle(), &semaphoreCreateInfo, nullptr, &vkSemaphore_);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateSemaphore" };
    }
}

auto Signal::value() const -> uint64_t
{
    assert(type_ == SignalType::TIMELINE);

    auto result = std::numeric_limits<uint64_t>::max();

    auto const& device = deviceRef_.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    if (auto vkResult = vkGetSemaphoreCounterValue(device.handle(), static_cast<VkSemaphore>(vkSemaphore_), &result);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkGetSemaphoreCounterValue" };
    }

    return result;
}

void Signal::signalFromCpu(uint64_t value)
{
    assert(type_ == SignalType::TIMELINE);

    auto const& device = deviceRef_.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    auto signalInfo = VkSemaphoreSignalInfo{};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
    signalInfo.semaphore = static_cast<VkSemaphore>(vkSemaphore_);
    signalInfo.value = value;

    if (auto vkResult = vkSignalSemaphore(device.handle(), &signalInfo); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkSignalSemaphore" };
    }
}

auto Signal::waitOnCpu(uint64_t value, uint64_t timeout) -> bool
{
    assert(type_ == SignalType::TIMELINE);

    auto const& device = deviceRef_.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    auto waitInfo = VkSemaphoreWaitInfo{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
    waitInfo.flags = VK_SEMAPHORE_WAIT_ANY_BIT;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &std::as_const(vkSemaphore_);
    waitInfo.pValues = &value;

    auto vkResult = VkResult{ VK_SUCCESS };
    if (vkResult = vkWaitSemaphores(device.handle(), &waitInfo, timeout);
        (vkResult != VK_SUCCESS && vkResult != VK_TIMEOUT)) {
        throw Exception{ vkResult, "vkWaitSemaphores" };
    }

    return vkResult == VK_SUCCESS;
}

}

#endif
