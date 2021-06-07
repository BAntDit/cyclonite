//
// Created by anton on 4/18/21.
//

#ifndef CYCLONITE_FRAMECOMMANDS_H
#define CYCLONITE_FRAMECOMMANDS_H

#include "compositor/passIterator.h"
#include "vulkan/baseCommandBufferSet.h"
#include "vulkan/buffer.h"
#include "vulkan/commandPool.h"

namespace cyclonite {
namespace compositor {
class Links;
}

class FrameCommands
{
public:
    FrameCommands();

    ~FrameCommands();

    void update(vulkan::Device& device,
                compositor::Links const& links,
                compositor::PassIterator const& begin,
                compositor::PassIterator const& end);

private:
    using graphics_queue_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    std::shared_ptr<vulkan::Buffer> indices_;
    std::shared_ptr<vulkan::Buffer> vertices_;
    std::shared_ptr<vulkan::Buffer> instances_;
    std::shared_ptr<vulkan::Buffer> commands_;
    std::shared_ptr<vulkan::Buffer> uniforms_;

    std::unique_ptr<graphics_queue_commands_t> graphicsCommands_;
};
}

#endif // CYCLONITE_FRAMECOMMANDS_H
