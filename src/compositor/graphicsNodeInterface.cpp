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
                                             is_surface_node_func_t surfaceNodeFunc,
                                             make_expired_func_t makeExpiredFunc,
                                             update_func_t updateFunc,
                                             end_func_t endFunc,
                                             write_frame_commands_func_t writeFrameCommandsFunc,
                                             frame_sync_func_t frameSyncFunc) noexcept
  : node_{ node }
  , dispose_{ disposeFunc }
  , begin_{ beginFunc }
  , waitStages_{ waitStagesFunc }
  , isSurfaceNode_{ surfaceNodeFunc }
  , makeExpired_{ makeExpiredFunc }
  , update_{ updateFunc }
  , end_{ endFunc }
  , writeFrameCommands_{ writeFrameCommandsFunc }
  , frameSync_{ frameSyncFunc }
{
}

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

auto GraphicsNodeInterface::begin(vulkan::Device& device) -> std::pair<VkSemaphore, size_t>
{
    return begin_(node_, device);
}

auto GraphicsNodeInterface::waitStages() -> std::pair<VkSemaphore*, VkPipelineStageFlags*>
{
    return waitStages_(node_);
}

void GraphicsNodeInterface::makeDescriptorSetExpired()
{
    makeExpired_(node_);
}

void GraphicsNodeInterface::update(uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime)
{
    update_(node_, semaphoreCount, frameNumber, deltaTime);
}

void GraphicsNodeInterface::end(uint32_t semaphoreCount)
{
    end_(node_, semaphoreCount);
}

void GraphicsNodeInterface::writeFrameCommands(vulkan::Device& device)
{
    writeFrameCommands_(node_, device);
}

auto GraphicsNodeInterface::frameSync(vulkan::Device& device, uint64_t frameNumber) -> size_t
{
    return frameSync_(node_, device, frameNumber);
}

void GraphicsNodeInterface::dispose()
{
    dispose_(node_);
}
}