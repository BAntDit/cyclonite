//
// Created by bantdit on 11/27/22.
//

#ifndef CYCLONITE_GRAPHICSNODEINTERFACE_H
#define CYCLONITE_GRAPHICSNODEINTERFACE_H

#include "typedefs.h"
#include <cstdint>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
class Device;
}

namespace cyclonite::compositor {
class BaseGraphicsNode;

class GraphicsNodeInterface
{
    using dispose_func_t = void (*)(void*);
    using begin_func_t = std::pair<VkSemaphore, size_t> (*)(void*, vulkan::Device&, uint64_t);
    using wait_stages_func_t = std::pair<VkSemaphore*, VkPipelineStageFlags*> (*)(void*);

public:
    GraphicsNodeInterface(void* node,
                          dispose_func_t disposeFunc,
                          begin_func_t beginFunc,
                          wait_stages_func_t waitStagesFunc) noexcept;

    GraphicsNodeInterface(GraphicsNodeInterface const&) = default;

    GraphicsNodeInterface(GraphicsNodeInterface&&) = default;

    ~GraphicsNodeInterface() = default;

    auto operator=(GraphicsNodeInterface const&) -> GraphicsNodeInterface& = default;

    auto operator=(GraphicsNodeInterface &&) -> GraphicsNodeInterface& = default;

    [[nodiscard]] auto get() const -> BaseGraphicsNode const&;
    auto get() -> BaseGraphicsNode&;

    [[nodiscard]] auto operator*() const -> BaseGraphicsNode const&;
    auto operator*() -> BaseGraphicsNode&;

    auto begin(vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t>;

    auto waitStages() -> std::pair<VkSemaphore*, VkPipelineStageFlags*>;

    void dispose();

private:
    void* node_;
    dispose_func_t dispose_;
    begin_func_t begin_;
    wait_stages_func_t waitStages_;
};
}

#endif // CYCLONITE_GRAPHICSNODEINTERFACE_H
