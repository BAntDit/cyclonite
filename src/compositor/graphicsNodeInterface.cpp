//
// Created by bantdit on 11/27/22.
//

#include "graphicsNodeInterface.h"
#include "baseGraphicsNode.h"
#include "vulkan/device.h"

namespace cyclonite::compositor {
GraphicsNodeInterface::GraphicsNodeInterface(void* node,
                                             dispose_func_t disposeFunc,
                                             begin_func_t beginFunc,
                                             wait_stages_func_t waitStagesFunc,
                                             is_surface_node_func_t surfaceNodeFunc) noexcept
  : node_{ node }
  , dispose_{ disposeFunc }
  , begin_{ beginFunc }
  , waitStages_{ waitStagesFunc }
  , isSurfaceNode_{ surfaceNodeFunc }
{}

auto GraphicsNodeInterface::get() const -> BaseGraphicsNode const&
{
    return *reinterpret_cast<BaseGraphicsNode const*>(node_);
}

auto GraphicsNodeInterface::get() -> BaseGraphicsNode&
{
    return *reinterpret_cast<BaseGraphicsNode*>(node_);
}

auto GraphicsNodeInterface::operator*() const -> BaseGraphicsNode const&
{
    return get();
}

auto GraphicsNodeInterface::operator*() -> BaseGraphicsNode&
{
    return get();
}

auto GraphicsNodeInterface::begin(vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t>
{
    return begin_(node_, device, frameNumber);
}

auto GraphicsNodeInterface::waitStages() -> std::pair<VkSemaphore*, VkPipelineStageFlags*>
{
    return waitStages_(node_);
}

void GraphicsNodeInterface::dispose()
{
    dispose_(node_);
}
}