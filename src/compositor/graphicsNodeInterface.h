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
    using is_surface_node_func_t = bool (*)();
    using make_expired_func_t = void (*)(void*, size_t);
    using update_func_t = void(*)(void*, uint32_t&, uint64_t, real);

public:
    GraphicsNodeInterface(void* node,
                          dispose_func_t disposeFunc,
                          begin_func_t beginFunc,
                          wait_stages_func_t waitStagesFunc,
                          is_surface_node_func_t surfaceNodeFunc,
                          make_expired_func_t makeExpiredFunc,
                          update_func_t updateFunc) noexcept;

    GraphicsNodeInterface(GraphicsNodeInterface const&) = default;

    GraphicsNodeInterface(GraphicsNodeInterface&&) = default;

    ~GraphicsNodeInterface() = default;

    auto operator=(GraphicsNodeInterface const&) -> GraphicsNodeInterface& = default;

    auto operator=(GraphicsNodeInterface &&) -> GraphicsNodeInterface& = default;

    [[nodiscard]] auto get() const -> BaseGraphicsNode const&;
    auto get() -> BaseGraphicsNode&;

    [[nodiscard]] auto operator*() const -> BaseGraphicsNode const&;
    auto operator*() -> BaseGraphicsNode&;

    [[nodiscard]] auto isSurfaceNode() const -> bool { return isSurfaceNode_(); }

    auto begin(vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t>;

    auto waitStages() -> std::pair<VkSemaphore*, VkPipelineStageFlags*>;

    void makeExpired(size_t bufferIndex);

    void update(uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime);

    void dispose();

private:
    void* node_;
    dispose_func_t dispose_;
    begin_func_t begin_;
    wait_stages_func_t waitStages_;
    is_surface_node_func_t isSurfaceNode_;
    make_expired_func_t makeExpired_;
    update_func_t update_;
};
}

#endif // CYCLONITE_GRAPHICSNODEINTERFACE_H
