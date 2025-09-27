//
// Created by anton on 9/27/25.
//

#ifndef CYCLONITE_VK_SIGNAL_H
#define CYCLONITE_VK_SIGNAL_H

#if defined(GFX_DRIVER_VULKAN)

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"

namespace cyclonite::gfx::vulkan {
class Signal : public core::ResourceBase
{
public:
    Signal(core::ResourceManagerBase* resourceManager,
           core::ResourceId resourceId,
           core::ResourceSharedRef deviceRef,
           SignalType signalType,
           uint64_t initialValue = 0);

    [[nodiscard]] auto type() const -> SignalType { return type_; }

    [[nodiscard]] auto handle() const -> VkSemaphore { return static_cast<VkSemaphore>(vkSemaphore_); }

    [[nodiscard]] auto value() const -> uint64_t;

    void signalFromCpu(uint64_t value);

    auto waitOnCpu(uint64_t value, uint64_t timeout) -> bool;

private:
    core::ResourceSharedRef deviceRef_;
    Handle<VkSemaphore> vkSemaphore_;
    SignalType type_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_SIGNAL_H