//
// Created by anton on 12/4/21.
//

#ifndef CYCLONITE_FRAMECOMMANDS_H
#define CYCLONITE_FRAMECOMMANDS_H

#include "vulkan/buffer.h"
#include "vulkan/device.h"

namespace cyclonite {
class BaseRenderTarget;
}

namespace cyclonite::compositor {
class Links;
class PassIterator;

class FrameCommands
{
private:
    FrameCommands() noexcept;

    explicit FrameCommands(size_t swapChainIndex) noexcept;

public:
    void update(vulkan::Device& device,
                BaseRenderTarget const& renderTarget,
                VkRenderPass vkRenderPass,
                Links const& links,
                PassIterator const& begin,
                PassIterator const& end);

private:
    using graphics_queue_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    size_t swapChainIndex_ = 0;

    std::shared_ptr<vulkan::Buffer> indices_;
    std::shared_ptr<vulkan::Buffer> vertices_;
    std::shared_ptr<vulkan::Buffer> instances_;
    std::shared_ptr<vulkan::Buffer> commands_;
    std::shared_ptr<vulkan::Buffer> uniforms_;

    uint32_t commandCount_;

    std::unique_ptr<graphics_queue_commands_t> graphicsCommands_;
};
}

#endif // CYCLONITE_FRAMECOMMANDS_H
