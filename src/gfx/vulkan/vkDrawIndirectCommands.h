//
// Created by anton on 11/3/26.
//

#ifndef CYCLONITE_GFX_VK_DRAW_INDIRECT_COMMANDS_H
#define CYCLONITE_GFX_VK_DRAW_INDIRECT_COMMANDS_H

#include <vector>

#include "core/resourceSharedRef.h"
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class DrawIndirectCommands
{
public:
    DrawIndirectCommands(core::ResourceSharedRef deviceRef, uint32_t initialCommandCount);

    [[nodiscard]] auto commandCount() const -> uint32_t { return commandCount_; }

private:
    core::ResourceSharedRef staging_;
    std::vector<VkDrawIndirectCommand> shadowBuffer_;
    VkDrawIndirectCommand* commandsPtr_;
    uint32_t commandCount_;
};
}

#endif // CYCLONITE_GFX_VK_DRAW_INDIRECT_COMMANDS_H
