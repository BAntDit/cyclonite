//
// Created by anton on 9/28/25.
//

#ifndef CYCLONITE_VK_COMMANDLIST_H
#define CYCLONITE_VK_COMMANDLIST_H

#include "core/resourceSharedRef.h"
#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <span>
#include <vector>

#if defined(GFX_DRIVER_VULKAN)
#include <unordered_map>
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

    void beginRenderPass(core::ResourceSharedRef const& renderPassRef);

    void bindPipeline(core::ResourceSharedRef const& pipelineRef);

    void bindDescriptorSet(PipelineBindPoint bindPoint,
                           core::ResourceSharedRef const& bindingSchemaRef,
                           core::ResourceSharedRef const& descriptorSetRef,
                           std::span<uint32_t> dynamicOffsets = {});

    void bindIndexBuffer(core::ResourceSharedRef const& bufferRef, size_t offset, IndexType indexType);

    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);

    void drawIndirect(core::ResourceSharedRef const& bufferRef, size_t offset, uint32_t count);

    void drawIndexedIndirect(core::ResourceSharedRef const& bufferRef, size_t offset, uint32_t count);

    void endRenderPass();

    void copyBuffers(core::ResourceSharedRef const& srcRef,
                     core::ResourceSharedRef const& dstRef,
                     size_t srcOffset,
                     size_t dstOffset,
                     size_t size);

    void acquireResourceForTransfer(PipelineStageFlagBits srcStageMask,
                                    PipelineStageFlagBits dstStageMask,
                                    AccessFlagBits srcAccessMask,
                                    AccessFlagBits dstAccessMask,
                                    core::ResourceSharedRef& resourceRef,
                                    size_t offset = 0,
                                    size_t size = std::numeric_limits<size_t>::max());

    void acquireResourceForGraphics(PipelineStageFlagBits srcStageMask,
                                    PipelineStageFlagBits dstStageMask,
                                    AccessFlagBits srcAccessMask,
                                    AccessFlagBits dstAccessMask,
                                    core::ResourceSharedRef& resourceRef,
                                    size_t offset = 0,
                                    size_t size = std::numeric_limits<size_t>::max());

    void releaseResourceToGraphics(PipelineStageFlagBits srcStageMask,
                                   PipelineStageFlagBits dstStageMask,
                                   AccessFlagBits srcAccessMask,
                                   AccessFlagBits dstAccessMask,
                                   core::ResourceSharedRef& resourceRef,
                                   size_t offset = 0,
                                   size_t size = std::numeric_limits<size_t>::max());

    void end();

    [[nodiscard]] auto handle() const -> VkCommandBuffer { return vkCommandBuffer_; }

private:
    void bufferMemoryBarrier(PipelineStageFlagBits srcStageMask,
                             PipelineStageFlagBits dstStageMask,
                             AccessFlagBits srcAccessMask,
                             AccessFlagBits dstAccessMask,
                             uint32_t srcQueueFamilyIndex,
                             uint32_t dstQueueFamilyIndex,
                             core::ResourceSharedRef const& bufferRef,
                             size_t offset,
                             size_t size);

    std::unordered_map<uint64_t, core::ResourceSharedRef> boundRefs_;
    core::ResourceWeakRef commandPool_;
    VkCommandBuffer vkCommandBuffer_;
    CommandListUsageFlagBits usage_;
    CommandListState state_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_COMMANDLIST_H