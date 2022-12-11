//
// Created by anton on 2/28/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
Workspace::Builder::Builder(resources::ResourceManager& resourceManager,
                            vulkan::Device& device,
                            size_t maxBytesPerNode /* = 64 * 1024*/,
                            uint8_t maxLogicNodeCount /* = 10*/,
                            uint8_t maxGraphicNodeCount /* = 10*/)
  : resourceManager_{ &resourceManager }
  , device_{ &device }
  , logicNodeOffsets_(maxLogicNodeCount, size_t{ 0 })
  , graphicNodeOffsets_(maxGraphicNodeCount, size_t{ 0 })
  , logicNodeSizes_(maxLogicNodeCount, std::size_t{ 0 })
  , graphicNodeSizes_(maxGraphicNodeCount, std::size_t{ 0 })
  , logicNodeStorage_(maxLogicNodeCount * maxBytesPerNode, std::byte{ 0 })
  , graphicNodeStorage_(maxGraphicNodeCount * maxBytesPerNode, std::byte{ 0 })
  , logicNodes_{}
  , graphicNodes_{}
  , logicNodeCount_{ 0 }
  , graphicNodeCount_{ 0 }
{}

auto Workspace::Builder::nodeCount(bool isLogic) const -> uint8_t
{
    return isLogic ? logicNodeCount_ : graphicNodeCount_;
}

auto Workspace::Builder::nodeOffset(bool isLogic, size_t index) const -> size_t
{
    assert((isLogic && index < logicNodeOffsets_.size()) || (!isLogic && index < graphicNodeOffsets_.size()));
    return isLogic ? logicNodeOffsets_[index] : graphicNodeOffsets_[index];
}

auto Workspace::Builder::nodeSize(bool isLogic, size_t index) const -> size_t
{
    assert((isLogic && index < logicNodeSizes_.size()) || (!isLogic && index < graphicNodeSizes_.size()));
    return isLogic ? logicNodeSizes_[index] : graphicNodeSizes_[index];
}

auto Workspace::Builder::nodeStorage(bool isLogic) const -> std::byte const*
{
    return isLogic ? logicNodeStorage_.data() : graphicNodeStorage_.data();
}

auto Workspace::Builder::nodeCount(bool isLogic) -> uint8_t&
{
    return isLogic ? logicNodeCount_ : graphicNodeCount_;
}

auto Workspace::Builder::nodeOffset(bool isLogic, size_t index) -> size_t&
{
    assert((isLogic && index < logicNodeOffsets_.size()) || (!isLogic && index < graphicNodeOffsets_.size()));
    return isLogic ? logicNodeOffsets_[index] : graphicNodeOffsets_[index];
}

auto Workspace::Builder::nodeSize(bool isLogic, size_t index) -> size_t&
{
    assert((isLogic && index < logicNodeSizes_.size()) || (!isLogic && index < graphicNodeSizes_.size()));
    return isLogic ? logicNodeSizes_[index] : graphicNodeSizes_[index];
}

auto Workspace::Builder::nodeStorage(bool isLogic) -> std::byte*
{
    return isLogic ? logicNodeStorage_.data() : graphicNodeStorage_.data();
}

auto Workspace::Builder::logicNodeNameToId(std::string_view _name) const -> uint64_t
{
    auto id = std::numeric_limits<uint64_t>::max();
    for (auto&& node : logicNodes_) {
        if (node.get().name() == _name) {
            id = node.get().id();
            break;
        }
    }

    return id;
}

auto Workspace::Builder::graphicsNodeNameToId(std::string_view _name) const -> uint64_t
{
    auto id = std::numeric_limits<uint64_t>::max();
    for (auto&& node : graphicNodes_) {
        if (node.get().name() == _name) {
            id = node.get().id();
            break;
        }
    }

    return id;
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

    for (auto i = uint8_t{ 0 }; i < nodeCount_; i++) {
        auto const& node = workspace.nodes_[i];

        for (auto const& [nodeIndex, sampler, _a, _b] : (*node).getInputs()) {
            (void)_a;
            (void)_b;
            (void)sampler;

            if (nodeIndex == std::numeric_limits<size_t>::max())
                continue;

            waitSemaphoreCount++; // waiting for previous node
        }

        waitSemaphoreCount += node.getExpectedWaitSignalCount();

        if (node.isSurfaceNode()) {
            // workspace can have only one presentation node
            assert(workspace.presentationNodeIndex_ == std::numeric_limits<uint8_t>::max());
            workspace.presentationNodeIndex_ = i;
        }
    }

    workspace.nodeWaitSemaphores_.resize(waitSemaphoreCount, VK_NULL_HANDLE);
    workspace.nodeDstStageMasks_.resize(waitSemaphoreCount, VK_PIPELINE_STAGE_FLAG_BITS_MAX_ENUM);

    workspace.nodeSignalSemaphores_.resize(workspace.nodes_.size(), VK_NULL_HANDLE);

    workspace.submits_.resize(workspace.nodes_.size(), VkSubmitInfo{});

    return workspace;
}
}
