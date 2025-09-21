//
// Created by anton on 9/7/25.
//

#ifndef CYCLONITE_VKCOMMANDPOOL_H
#define CYCLONITE_VKCOMMANDPOOL_H

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"
#include <atomic>
#include <thread>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class CommandPool : public core::ResourceBase
{
public:
    CommandPool(core::ResourceManagerBase* resourceManager,
                core::ResourceId resourceId,
                core::ResourceWeakRef deviceRef,
                uint32_t queueFamilyIndex,
                CommandPoolFlagBits flags);

    ~CommandPool() = default;

    [[nodiscard]] auto queueFamilyIndex() const -> uint32_t { return queueFamilyIndex_; }

private:
    core::ResourceSharedRef deviceRef_;
    Handle<VkCommandPool> vkCommandPool_;
    std::thread::id threadId_;
    uint32_t queueFamilyIndex_;
    CommandPoolFlagBits flags_;
    std::atomic<uint32_t> commandBufferInUseCount_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKCOMMANDPOOL_H
