//
// Created by anton on 11/25/21.
//

#include "nodeInterface.h"
#include "baseNode.h"

namespace cyclonite::compositor {
NodeInterface::NodeInterface(void* node,
                             make_expired_func_t makeExpiredFunc,
                             get_expected_wait_signals_func_t getExpectedWaitSignalsFunc,
                             is_surface_node_func_t isSurfaceNodeFunc,
                             begin_func_t beginFunc,
                             update_func_t updateFunc,
                             end_func_t endFunc,
                             get_current_frame_func_t getFrameFunc,
                             dispose_func_t disposeFunc,
                             write_frame_commands_func_t writeFrameCommandsFunc) noexcept
  : node_{ node }
  , makeExpired_{ makeExpiredFunc }
  , getExpectedWaitSignalCount_{ getExpectedWaitSignalsFunc }
  , isSurfaceNode_{ isSurfaceNodeFunc }
  , begin_{ beginFunc }
  , update_{ updateFunc }
  , end_{ endFunc }
  , getFrame_{ getFrameFunc }
  , dispose_{ disposeFunc }
  , writeFrameCommands_{ writeFrameCommandsFunc }
{
    assert(node_ != nullptr);
}

void NodeInterface::makeExpired(size_t index)
{
    makeExpired_(node_, index);
}

auto NodeInterface::getExpectedWaitSignalCount() const -> uint32_t
{
    return getExpectedWaitSignalCount_();
}

auto NodeInterface::isSurfaceNode() const -> bool
{
    return isSurfaceNode_();
}

auto NodeInterface::begin(vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t>
{
    return begin_(node_, device, frameNumber);
}

void NodeInterface::update(uint32_t& signalCount, VkSemaphore* baseSignal, VkPipelineStageFlags* baseFlag)
{
    return update_(node_, signalCount, baseSignal, baseFlag);
}

void NodeInterface::end(vulkan::Device& device)
{
    return end_(node_, device);
}

void NodeInterface::dispose()
{
    dispose_(node_);
}

auto NodeInterface::get() -> BaseNode&
{
    return *(reinterpret_cast<BaseNode*>(node_));
}

auto NodeInterface::get() const -> BaseNode const&
{
    return *(reinterpret_cast<BaseNode const*>(node_));
}

auto NodeInterface::operator*() -> BaseNode&
{
    return get();
}

auto NodeInterface::operator*() const -> BaseNode const&
{
    return get();
}

auto NodeInterface::getCurrentFrame() -> FrameCommands&
{
    return getFrame_(node_);
}

auto NodeInterface::passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&
{
    return get().passFinishedSemaphore();
}

auto NodeInterface::getInputs() const -> Links const&
{
    return std::as_const(*this).getInputs();
}

auto NodeInterface::getInputs() -> Links&
{
    return get().getInputs();
}

void NodeInterface::writeFrameCommands(vulkan::Device& device)
{
    return writeFrameCommands_(node_, device);
}
}