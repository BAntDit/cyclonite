//
// Created by anton on 11/3/26.
//

#ifndef CYCLONITE_GFX_VK_DRAW_INDIRECT_COMMANDS_H
#define CYCLONITE_GFX_VK_DRAW_INDIRECT_COMMANDS_H

#include <vector>
#include <vulkan/vulkan.h>
#include "core/resourceSharedRef.h"
#include "gfx/common.h"

namespace cyclonite::gfx::vulkan {
class DrawIndirectCommands
{
public:
    DrawIndirectCommands(core::ResourceSharedRef const& deviceRef, uint32_t initialCommandCount);

    [[nodiscard]] auto commandCount() const -> uint32_t { return commandCount_; }
    
    [[nodiscard]] auto capacity() const -> uint32_t { return shadowBuffer_.capacity(); }
    
    [[nodiscard]] auto state() const -> DrawIndirectCommandState { return state_; }
    
    [[nodiscard]] auto state() -> DrawIndirectCommandState& { return state_; }
    
    [[nodiscard]] auto staging() const -> core::ResourceSharedRef const& { return stagingRef_; }

    [[nodiscard]] auto vertexCount(size_t commandIndex) const -> uint32_t;
    
    [[nodiscard]] auto vertexCount(size_t commandIndex) -> uint32_t&;

    [[nodiscard]] auto firstVertex(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto firstVertex(size_t commandIndex) -> uint32_t&;

    [[nodiscard]] auto instanceCount(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto instanceCount(size_t commandIndex) -> uint32_t&;

    [[nodiscard]] auto firstInstance(size_t commandIndex) const -> uint32_t;

    [[nodiscard]] auto firstInstance(size_t commandIndex) -> uint32_t&;

    void addCommand(uint32_t vertexCount, uint32_t firstVertex, uint32_t instanceCount, uint32_t firstInstance);

    void validateStaging();

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef stagingRef_;
    std::vector<VkDrawIndirectCommand> shadowBuffer_;
    VkDrawIndirectCommand* commandsPtr_;
    uint32_t commandCount_;
    DrawIndirectCommandState state_;
};
}

#endif // CYCLONITE_GFX_VK_DRAW_INDIRECT_COMMANDS_H
