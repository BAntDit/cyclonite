//
// Created by anton on 1/10/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
Workspace::Workspace() noexcept
  : logicNodeCount_{ 0 }
  , logicNodeStorage_{}
  , logicNodes_{}
  , idToLogicNodeIndex_{}
  , nameToLogicNodeIndex_{}
  , graphicsNodeCount_{ 0 }
  , graphicsNodeStorage_{}
  , graphicsNodes_{}
  , idToGraphicsNodeIndex_{}
  , nameToGraphicsNodeIndex_{}
  , frameNumber_{ 0 }
  , submits_{}
  , gfxFutures_{}
  , submitCount_{ 0 }
  , frameFences_{}
  , lastTimeUpdate_{ std::chrono::high_resolution_clock::now() }
{}

void Workspace::render(vulkan::Device& device)
{
    beginFrame();

    // TODO:: receive as argument instead
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

    // frame sync
    auto fence = syncFrame(device);

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
                [&node, &device]() -> std::pair<VkSemaphore, size_t> {
                    return node.begin(device);
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
                                    node.makeDescriptorSetExpired();
                                }
                            }
                        } // input semantics
                    }     // all inputs
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
    endFrame(device, fence);
}

auto Workspace::syncFrame(vulkan::Device& device) -> VkFence
{
    auto frameSyncTask = [& fences = frameFences_,
                          &nodes = graphicsNodes_,
                          nodeCount = graphicsNodeCount_,
                          frameNumber = frameNumber_,
                          &device]() -> VkFence {
        auto vkFence = VkFence{ VK_NULL_HANDLE };
        auto fenceIdx = std::numeric_limits<size_t>::max();

        for (auto i = uint8_t{ 0 }; i < nodeCount; i++) {
            auto&& node = nodes[i];
            auto syncIdx = node.frameSync(device, frameNumber);

            if (node.isSurfaceNode()) {
                fenceIdx = syncIdx;
            }
        }

        if (fenceIdx != std::numeric_limits<size_t>::max()) {
            auto const& fence = fences[fenceIdx];

            if (auto result =
                  vkWaitForFences(device.handle(), 1, &fence, VK_FALSE, std::numeric_limits<uint64_t>::max());
                result != VK_SUCCESS) {
                throw std::runtime_error("can not sync frame");
            }

            if (auto result = vkResetFences(device.handle(), 1, &fence); result != VK_SUCCESS) {
                throw std::runtime_error("can not reset frame sync fence");
            }

            vkFence = static_cast<VkFence>(fence);
        }

        return vkFence;
    };

    auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(frameSyncTask);
    return future.get();
}

void Workspace::beginFrame()
{
    submitCount_ = 0;
}

void Workspace::endFrame(vulkan::Device& device, VkFence fence)
{
    for (auto i = size_t{ 0 }; i < submitCount_; i++) {
        gfxFutures_[i].get();
        submits_[i] = graphicsNodes_[i].get().submitInfo();
    }

    auto endFrameTask = [&device,
                         &submits = submits_,
                         &graphicsNodes = graphicsNodes_,
                         submitCount = submitCount_,
                         graphicsNodeCount = graphicsNodeCount_,
                         fence]() -> void {
        if (auto result = vkQueueSubmit(device.graphicsQueue(), submitCount, submits.data(), fence);
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

auto Workspace::get(uint64_t id) const -> Node const&
{
    if (idToGraphicsNodeIndex_.contains(id)) {
        auto idx = idToGraphicsNodeIndex_.at(id);
        return graphicsNodes_[idx].get();
    } else if (idToLogicNodeIndex_.contains(id)) {
        auto idx = idToLogicNodeIndex_.at(id);
        return logicNodes_[idx].get();
    }

    assert(false);
    std::terminate();
}

auto Workspace::get(uint64_t id) -> Node&
{
    return const_cast<Node&>(std::as_const(*this).get(id));
}

auto Workspace::get(std::string_view name) const -> Node const&
{
    if (nameToGraphicsNodeIndex_.contains(name.data())) {
        auto idx = nameToGraphicsNodeIndex_.at(name.data());
        return graphicsNodes_[idx].get();
    } else if (nameToLogicNodeIndex_.contains(name.data())) {
        auto idx = nameToLogicNodeIndex_.at(name.data());
        return logicNodes_[idx].get();
    }

    assert(false);
    std::terminate();
}

auto Workspace::get(std::string_view name) -> Node&
{
    return const_cast<Node&>((std::as_const(*this)).get(name));
}

Workspace::~Workspace()
{
    for (auto&& n : graphicsNodes_)
        n.dispose();

    for (auto&& n : logicNodes_)
        n.dispose();
}
}
