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
class BaseGraphicsNode;

class FrameCommands
{
    friend class BaseGraphicsNode;

private:
    FrameCommands() noexcept;

    explicit FrameCommands(size_t swapChainIndex) noexcept;

public:
    void update(vulkan::Device& device,
                BaseRenderTarget const& renderTarget,
                VkRenderPass vkRenderPass,
                Links const& links,
                PassIterator const& begin,
                PassIterator const& end,
                bool isExpired);

    void setIndexBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& indices);

    void setVertexBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& vertices);

    void setInstanceBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& instances);

    void setCommandBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& commands, uint32_t commandCount);

    void setUniformBuffer(VkQueue graphicQueue, std::shared_ptr<vulkan::Buffer> const& uniforms);

private:
    void _resetCommands(VkQueue graphicQueue);

    using graphics_queue_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    size_t bufferIndex_;

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
