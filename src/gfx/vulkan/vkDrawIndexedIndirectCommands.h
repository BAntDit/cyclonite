
#ifndef CYCLONITE_GFX_VK_DRAW_INDEXED_INDIRECT_COMMANDS_H
#define CYCLONITE_GFX_VK_DRAW_INDEXED_INDIRECT_COMMANDS_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <vector>
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class DrawIndexedIndirectCommands
{
public:
    DrawIndexedIndirectCommands() = default;

    DrawIndexedIndirectCommands(core::ResourceSharedRef const& deviceRef, uint32_t initialCommandCount);

    [[nodiscard]] auto commandCount() const -> uint32_t { return commandCount_; }

    [[nodiscard]] auto capacity() const -> uint32_t { return shadowBuffer_.capacity(); }

    [[nodiscard]] auto state() const -> DrawIndirectCommandState { return state_; }

    [[nodiscard]] auto state() -> DrawIndirectCommandState& { return state_; }

    [[nodiscard]] auto staging() const -> core::ResourceSharedRef const& { return stagingRef_; }

    [[nodiscard]] auto indexCount(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto indexCount(size_t commandIndex) -> uint32_t&;

    [[nodiscard]] auto firstIndex(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto firstIndex(size_t commandIndex) -> uint32_t&;

    [[nodiscard]] auto vertexOffset(size_t commandIndex) const -> int32_t;

    [[nodiscard]] auto vertexOffset(size_t commandIndex) -> int32_t&;

    [[nodiscard]] auto instanceCount(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto instanceCount(size_t commandIndex) -> uint32_t&;

    [[nodiscard]] auto firstInstance(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto firstInstance(size_t commandIndex) -> uint32_t&;

    void addCommand(uint32_t indexCount,
                    uint32_t firstIndex,
                    int32_t vertexOffset,
                    uint32_t instanceCount,
                    uint32_t firstInstance);

    void validateStaging();

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef stagingRef_;
    std::vector<VkDrawIndexedIndirectCommand> shadowBuffer_;
    VkDrawIndexedIndirectCommand* commandsPtr_;
    uint32_t commandCount_;
    DrawIndirectCommandState state_;
};
}

#endif // CYCLONITE_GFX_VK_DRAW_INDEXED_INDIRECT_COMMANDS_H
