//
// Created by anton on 1/10/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
Workspace::Workspace() noexcept
  : frameNumber_{ 0 }
  , frameIndex_{ 0 }
  , swapChainLength_{ 0 }
  , nodeCount_{ 0 }
  , nodeStorage_{}
  , nodes_{}
  , nodeDstStageMasks_{}
  , nodeWaitSemaphores_{}
  , nodeSignalSemaphores_{}
  , submits_{}
{}

void Workspace::render(vulkan::Device& device)
{
    auto semaphoreOffset = size_t{ 0 };
    auto submitCount = uint32_t{ 0 };

    for (auto index = uint8_t{ 0 }, count = nodeCount_; index < count; index++) {
        assert(semaphoreOffset < nodeWaitSemaphores_.size() && semaphoreOffset < nodeDstStageMasks_.size());

        auto& node = nodes_[index];
        auto& inputs = node.getInputs();

        auto [swapChainSemaphore, commandsIndex] = node.begin(device, frameNumber_);

        auto semaphoreCount = uint32_t{ 0 };
        auto* baseSemaphore = nodeWaitSemaphores_.data() + semaphoreOffset;
        auto* baseDstStageMask = nodeDstStageMasks_.data() + semaphoreOffset;

        if (swapChainSemaphore != VK_NULL_HANDLE) { // waiting for acquired image (frame buffer)
            *(baseSemaphore + semaphoreCount) = swapChainSemaphore;
            *(baseDstStageMask + semaphoreCount) = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            semaphoreCount++;
        }

        for (size_t linkIdx = 0, linkCount = inputs.size(); linkIdx < linkCount; linkIdx++) {
            auto& [inputLinkIdx, sampler, views, semantics] = inputs.get(linkIdx);

            (void)sampler;

            if (inputLinkIdx == std::numeric_limits<size_t>::max())
                continue;

            auto const& inputNode = nodes_[inputLinkIdx];

            auto signal = static_cast<VkSemaphore>(inputNode.passFinishedSemaphore());

            if (signal != VK_NULL_HANDLE) { // wait all nodes this node depends on
                *(baseSemaphore + semaphoreCount) = signal;
                *(baseDstStageMask + semaphoreCount) = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                semaphoreCount++;
            }

            for (auto i = size_t{ 0 }; i < value_cast(RenderTargetOutputSemantic::COUNT); i++) {
                auto const& rt =
                  inputNode.isSurfaceNode()
                    ? static_cast<BaseRenderTarget const&>((*inputNode).getRenderTarget<SurfaceRenderTarget>())
                    : static_cast<BaseRenderTarget const&>((*inputNode).getRenderTarget<FrameBufferRenderTarget>());

                auto semantic = semantics[i];

                if (semantic != RenderTargetOutputSemantic::INVALID) {
                    auto& view = views[i];
                    auto const& attachment = rt.getColorAttachment(commandsIndex, semantic);

                    if (view != attachment.handle()) {
                        view = attachment.handle();
                        node.makeExpired(commandsIndex);
                    }
                }
            } // all input's views
        }

        node.update(semaphoreCount, baseSemaphore, baseDstStageMask);

        semaphoreOffset += semaphoreCount;

        auto& submit = submits_[submitCount++] = {};

        // TODO:: NODE END
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.waitSemaphoreCount = semaphoreCount;
        submit.pWaitSemaphores = baseSemaphore;
        submit.pWaitDstStageMask = baseDstStageMask;
        // command buffers
        auto const signalCount = uint32_t{ 1 };

        submit.signalSemaphoreCount = signalCount;
        submit.pSignalSemaphores = &(*std::as_const(node)).passFinishedSemaphore();

        node.end(submit); // do there the render system do in the end - send pass finished semaphore
    }

    if (auto result = vkQueueSubmit(device.graphicsQueue(), submitCount, submits_.data(), VK_NULL_HANDLE);
        result != VK_SUCCESS) {
        throw std::runtime_error{ "submit commands failed" };
    }

    /* auto frameFence = static_cast<VkFence>(frameFences_[frameIndex_]);

    // TODO:: semaphore instead of fence
    // wait until frame over before render this frame
    vkWaitForFences(device.handle(), 1, &frameFence, VK_TRUE, std::numeric_limits<uint64_t>::max());

    for (auto&& node : nodes_) {
        node->render(frameIndex_, frameFence);
    }

    // back to unsignaled state
    vkResetFences(device.handle(), 1, &frameFence);

    assert(swapChainLength_ != 0);

    frameIndex_ = frameNumber_++ % swapChainLength_;*/
}

Workspace::~Workspace()
{
    for (auto&& node : nodes_)
        node.dispose();
}
}
