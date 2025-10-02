//
// Created by anton on 9/7/25.
//

#ifndef CYCLONITE_VKCOMMANDPOOL_H
#define CYCLONITE_VKCOMMANDPOOL_H

#include "core/refFromThisMixin.h"
#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/commandList.h"
#include "gfx/common.h"
#include "handle.h"
#include <thread>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class CommandPool
  : public core::ResourceBase
  , public core::EnableRefFromThis
{
public:
    CommandPool(core::ResourceManagerBase* resourceManager,
                core::ResourceId resourceId,
                core::ResourceSharedRef deviceRef,
                uint32_t queueFamilyIndex,
                CommandPoolFlagBits flags);

    ~CommandPool() = default;

    [[nodiscard]] auto queueFamilyIndex() const -> uint32_t { return queueFamilyIndex_; }

    [[nodiscard]] auto allocCommandList() -> gfx::CommandList;

    [[nodiscard]] auto handle() const -> VkCommandPool { return static_cast<VkCommandPool>(vkCommandPool_); }

    [[nodiscard]] auto device() const -> core::ResourceSharedRef { return deviceRef_; }

    void reset(bool releasePoolResources = false);

private:
    core::ResourceSharedRef deviceRef_;
    Handle<VkCommandPool> vkCommandPool_;
    std::thread::id threadId_;
    uint32_t queueFamilyIndex_;
    CommandPoolFlagBits flags_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VKCOMMANDPOOL_H
