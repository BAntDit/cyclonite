//
// Created by anton on 1/10/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
Workspace::Workspace() noexcept
  : frameNumber_{ 0 }
  , nodeDstStageMasks_{}
  , nodeWaitSemaphores_{}
  , nodeSignalSemaphores_{}
  , submits_{}
  , submitCount_{ 0 }
  , presentationNodeIndex_{ std::numeric_limits<uint8_t>::max() }
  , lastTimeUpdate_{ std::chrono::high_resolution_clock::now() }
{}

void Workspace::render(vulkan::Device& device)
{
    beginFrame();

    auto updateTime = std::chrono::high_resolution_clock::now();
    auto dt = std::min(std::chrono::duration<real, std::ratio<1>>{ updateTime - lastTimeUpdate_ }.count(), 0.1f);

    lastTimeUpdate_ = updateTime;

    // logic nodes update
    for (auto lni = uint8_t{ 0 }; lni < logicNodeCount_; lni++) {
        auto& node = logicNodes_[lni];

        auto future = std::shared_future<void>{ multithreading::Worker::threadWorker().submitTask(
          [frameNumber = frameNumber_, node = node /* just copy interface */, dt]() mutable -> void {
              node.get().resolveDependencies();
              node.update(frameNumber, dt);
          }) };

        // further nodes
        for (auto j = lni; j < logicNodeCount_; j++) {
            auto& n = logicNodes_[j];
            auto id = node.get().id();

            if (n.get().dependsOn(id)) {
                n.get().updateDependency(id, future);
            }
        }

        for (auto k = uint8_t{ 0 }; k < graphicsNodeCount_; k++) {
            auto& gn = graphicsNodes_[k];
            auto id = node.get().id();

            if (gn.get().dependsOn(id)) {
                gn.get().updateDependency(id, future);
            }
        }
    }

    for (auto gni = uint8_t{ 0 }; gni < graphicsNodeCount_; gni++) {
        auto& node = graphicsNodes_[gni];

        // it's going to be not trivial thing to execute gfx node in the parallel
        // but let's try
        auto future = std::shared_future<void>{ multithreading::Worker::threadWorker().submitTask(
          [& graphicsNodes = graphicsNodes_,                   // read only
             & idToGraphicsNodeIndex = idToGraphicsNodeIndex_, // read only
           frameNumber = frameNumber_,                         // read only
           node = node,                                        // just copy interface
           dt,
           &device]() mutable -> void {
              node.get().resolveDependencies();

              // safe
              auto semaphoreCount = uint32_t{ 0 };
              auto [baseSemaphore, baseDstStageMask] = node.waitStages();

              // node::begin modifies frame buffer index and have to be in the render thread
              // to avoid data races
              auto beginFuture = multithreading::Worker::threadWorker().taskManager().submitRenderTask(
                [&node, &device, frameNumber]() -> std::pair<VkSemaphore, size_t> {
                    return node.begin(device, frameNumber);
                });
              auto [renderTargetReadySemaphore, commandIndex] = beginFuture.get();

              if (renderTargetReadySemaphore != VK_NULL_HANDLE) { // to waiting for acquired image or frame buffer
                  *(baseSemaphore + semaphoreCount) = renderTargetReadySemaphore;
                  *(baseDstStageMask + semaphoreCount) = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                  semaphoreCount++;
              }

              // reads input frame buffer index and must be in the render thread
              auto inputUpdateFuture = multithreading::Worker::threadWorker().taskManager().submitRenderTask(
                [&node,
                 &graphicsNodes,
                 &idToGraphicsNodeIndex,
                 &semaphoreCount,
                 baseSemaphore = baseSemaphore,
                 baseDstStageMask = baseDstStageMask]() -> void {
                    auto& inputs = node.get().inputs();

                    for (size_t linkIdx = 0, linkCount = inputs.size(); linkIdx < linkCount; linkIdx++) {
                        auto& [inputNodeId, sampler, views, semantics] = inputs.get(linkIdx);
                        (void)sampler;

                        if (inputNodeId == std::numeric_limits<size_t>::max())
                            continue;

                        assert(idToGraphicsNodeIndex.contains(inputNodeId));
                        auto const& inputNode = graphicsNodes[idToGraphicsNodeIndex[inputNodeId]];

                        auto signal = inputNode.get().passFinishedSemaphore();
                        if (signal != VK_NULL_HANDLE) { // to wait all nodes this node depends on
                            *(baseSemaphore + semaphoreCount) = signal;
                            *(baseDstStageMask + semaphoreCount) = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                            semaphoreCount++;
                        }

                        auto const inputFrameBufferIndex = inputNode.get().frameBufferIndex();
                        for (auto i = size_t{ 0 }; i < value_cast(RenderTargetOutputSemantic::COUNT); i++) {
                            auto const& rt = inputNode.isSurfaceNode()
                                               ? static_cast<BaseRenderTarget const&>(
                                                   (*inputNode).getRenderTarget<SurfaceRenderTarget>())
                                               : static_cast<BaseRenderTarget const&>(
                                                   (*inputNode).getRenderTarget<FrameBufferRenderTarget>());

                            auto semantic = semantics[i];

                            if (semantic != RenderTargetOutputSemantic::INVALID) {
                                auto& view = views[i];
                                auto const& attachment = rt.getColorAttachment(inputFrameBufferIndex, semantic);

                                if (view != attachment.handle()) {
                                    view = attachment.handle();
                                    node.makeExpired(commandIndex);
                                }
                            }
                        } // input semantics
                    } // all inputs
                });
              inputUpdateFuture.get();

              // it has to be safe after dependency resolve
              node.update(semaphoreCount, frameNumber, dt);

              // writes own submit info only
              node.end(semaphoreCount);
          }) };

        // further nodes
        for (auto j = gni; j < graphicsNodeCount_; j++) {
            auto& n = graphicsNodes_[j];
            auto id = node.get().id();

            if (n.get().dependsOn(id)) {
                n.get().updateDependency(id, future);
            }
        }

        assert(submitCount_ < gfxFutures_.size());
        gfxFutures_[submitCount_++] = future;
    } // gfx nodes

    // submits everything and swap buffers
    endFrame(device);
}

void Workspace::beginFrame()
{
    submitCount_ = 0;
}

void Workspace::endFrame(vulkan::Device& device)
{
    for (auto i = size_t{ 0 }; i < submitCount_; i++) {
        gfxFutures_[i].get();
        submits_[i] = graphicsNodes_[i].get().submitInfo();
    }

    auto endFrameTask = [&device,
                         &submits = submits_,
                         &graphicsNodes = graphicsNodes_,
                         submitCount = submitCount_,
                         graphicsNodeCount = graphicsNodeCount_]() -> void {
        if (auto result = vkQueueSubmit(device.graphicsQueue(), submitCount, submits.data(), VK_NULL_HANDLE);
            result != VK_SUCCESS) {
            throw std::runtime_error{ "submit commands failed" };
        }

        for (auto gni = uint8_t{ 0 }; gni < graphicsNodeCount; gni++) {
            auto& node = graphicsNodes[gni];
            node.get().swapBuffers(device);
            node.get().clearDependencies();
        }
    };

    if (multithreading::Render::isInRenderThread()) {
        endFrameTask();
    } else {
        assert(multithreading::Worker::isInWorkerThread());
        auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(endFrameTask);
        future.get();
    }

    frameNumber_++;
}

Workspace::~Workspace()
{
    // for (auto&& node : nodes_)
    //    node.dispose();
}
}
