//
// Created by anton on 2/28/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
Workspace::Builder::Builder(vulkan::Device& device,
                            uint8_t maxNodeCount /* = 10*/,
                            size_t maxBytesPerNode /* = 64 * 1024*/)
  : device_{ &device }
  , waitSemaphoresPerNodeCount_{}
  , nodeSignalSemaphores_{}
  , nodeWaitSemaphores_{}
  , nodeCount_{ 0 }
  , nodeStorage_{ maxNodeCount * maxBytesPerNode, std::byte{ 0 } }
  , nodeSizes_{ maxNodeCount, size_t{ 0 } }
  , nodeOffsets_{ maxNodeCount, size_t{ 0 } }
  , nodes_{}
{
    nodes_.reserve(maxNodeCount);
    nodeTypeIds_.reserve(maxNodeCount);
}

auto Workspace::Builder::allocateNode(size_t size) -> void*
{
    auto offset = (nodeCount_ > 0) ? (nodeOffsets_[nodeCount_ - 1] + nodeSizes_[nodeCount_ - 1]) : size_t{ 0 };

    auto align = size_t{ 64 };
    auto alignedSize = size + (((size % align) > 0) ? (align - (size % align)) : size_t{ 0 });

    nodeSizes_[nodeCount_] = alignedSize;
    nodeOffsets_[nodeCount_] = offset;

    nodeCount_++;

    return nodeStorage_.data() + offset;
}

auto Workspace::Builder::build() -> Workspace
{
    Workspace workspace{};

    workspace.nodeCount_ = nodeCount_;

    [[maybe_unused]] auto const* storageBase = nodeStorage_.data();
    workspace.nodeStorage_ = std::move(nodeStorage_);
    assert(workspace.nodeStorage_.data() == storageBase);

    workspace.nodes_ = std::move(nodes_);
    workspace.nodeTypeIds_ = std::move(nodeTypeIds_);

    uint32_t waitSemaphoreCount = 0;

    for (auto const& node : workspace.nodes_) {
        for (auto const& [nodeIndex, sampler, _a, _b] : (*node).getInputs()) {
            (void)_a;
            (void)_b;
            (void)sampler;

            if (nodeIndex == std::numeric_limits<size_t>::max())
                continue;

            waitSemaphoreCount++; // waiting for previous node
        }

        waitSemaphoreCount += node.getExpectedWaitSignalCount();
    }

    workspace.nodeWaitSemaphores_.resize(waitSemaphoreCount, VK_NULL_HANDLE);
    workspace.nodeDstStageMasks_.resize(waitSemaphoreCount, VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);

    workspace.nodeSignalSemaphores_.resize(workspace.nodes_.size(), VK_NULL_HANDLE);

    /*workspace.submits_.resize(nodes.size(), VkSubmitInfo{});
     */

    return workspace;
}
}
