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
  : core::ResourceBase{ resourceManager, resourceId, true }
  , deviceRef_{ deviceRef }
  , vkSemaphore_{ deviceRef.as<gfx::type_traits::platform_implementation_t<gfx::Device>>().handle(),
                  vkDestroySemaphore }
  , type_{ signalType }
{
    assert(deviceRef.valid());
    auto& device = deviceRef.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    auto semaphoreTypeCreateInfo = VkSemaphoreTypeCreateInfoKHR{};
    semaphoreTypeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO_KHR;
    semaphoreTypeCreateInfo.semaphoreType =
      signalType == SignalType::BINARY ? VK_SEMAPHORE_TYPE_BINARY_KHR : VK_SEMAPHORE_TYPE_TIMELINE_KHR;
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

    if (auto vkResult = vkGetSemaphoreCounterValueKHR(device.handle(), static_cast<VkSemaphore>(vkSemaphore_), &result);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkGetSemaphoreCounterValueKHR" };
    }

    return result;
}

void Signal::signalFromCpu(uint64_t value)
{
    assert(type_ == SignalType::TIMELINE);

    auto const& device = deviceRef_.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    auto signalInfo = VkSemaphoreSignalInfoKHR{};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO_KHR;
    signalInfo.semaphore = static_cast<VkSemaphore>(vkSemaphore_);
    signalInfo.value = value;

    if (auto vkResult = vkSignalSemaphoreKHR(device.handle(), &signalInfo); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkSignalSemaphoreKHR" };
    }
}

auto Signal::waitOnCpu(uint64_t value, uint64_t timeout) -> bool
{
    assert(type_ == SignalType::TIMELINE);

    auto const& device = deviceRef_.as<gfx::type_traits::platform_implementation_t<gfx::Device>>();

    auto waitInfo = VkSemaphoreWaitInfoKHR{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO_KHR;
    waitInfo.flags = VK_SEMAPHORE_WAIT_ANY_BIT_KHR;
    waitInfo.semaphoreCount = 1;
    waitInfo.pSemaphores = &std::as_const(vkSemaphore_);
    waitInfo.pValues = &value;

    auto vkResult = VkResult{ VK_SUCCESS };
    if (vkResult = vkWaitSemaphoresKHR(device.handle(), &waitInfo, timeout);
        (vkResult != VK_SUCCESS && vkResult != VK_TIMEOUT)) {
        throw Exception{ vkResult, "vkWaitSemaphoresKHR" };
    }

    return vkResult == VK_SUCCESS;
}

}

#endif
