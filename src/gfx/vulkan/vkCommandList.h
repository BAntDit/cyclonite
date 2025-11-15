//
// Created by anton on 9/28/25.
//

#ifndef CYCLONITE_VK_COMMANDLIST_H
#define CYCLONITE_VK_COMMANDLIST_H

#include "core/resourceSharedRef.h"
#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <vector>

#if defined(GFX_DRIVER_VULKAN)
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class CommandPool;

class CommandList
{
    friend class CommandPool;

public:
    explicit CommandList(core::ResourceWeakRef commandPool);

    [[nodiscard]] auto state() const -> CommandListState { return state_; }

    [[nodiscard]] auto usage() const -> CommandListUsageFlagBits { return usage_; }

    void begin(CommandListUsageFlagBits usage);

    void beginRenderPass(core::ResourceSharedRef renderPassRef);

    void bindPipeline(core::ResourceSharedRef pipelineRef);

    void endRenderPass();

    void end();

    [[nodiscard]] auto handle() const -> VkCommandBuffer { return vkCommandBuffer_; }

private:
    std::vector<core::ResourceSharedRef> boundRefs_;
    core::ResourceWeakRef commandPool_;
    VkCommandBuffer vkCommandBuffer_;
    CommandListUsageFlagBits usage_;
    CommandListState state_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_COMMANDLIST_H