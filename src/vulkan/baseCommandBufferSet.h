//
// Created by bantdit on 2/14/20.
//

#ifndef CYCLONITE_BASECOMMANDBUFFERSET_H
#define CYCLONITE_BASECOMMANDBUFFERSET_H

#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
class BaseCommandBufferSet
{
public:
    BaseCommandBufferSet() = default;

    virtual ~BaseCommandBufferSet() = default;

    [[nodiscard]] virtual auto getCommandBuffer(size_t) const -> VkCommandBuffer = 0;

    [[nodiscard]] virtual auto commandBufferCount() const -> size_t = 0;

    [[nodiscard]] virtual auto pCommandBuffers() const -> VkCommandBuffer const* = 0;
};
}

#endif // CYCLONITE_BASECOMMANDBUFFERSET_H
