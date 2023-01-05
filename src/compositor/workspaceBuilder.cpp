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

auto Workspace::Builder::nodeNameToId(std::string_view _name) const -> uint64_t
{
    auto id = logicNodeNameToId(_name);
    return id == std::numeric_limits<uint64_t>::max() ? graphicsNodeNameToId(_name) : id;
}

auto Workspace::Builder::build() -> Workspace
{
    Workspace workspace{};

    workspace.logicNodeCount_ = logicNodeCount_;

    [[maybe_unused]] auto const* logicStorageBase = logicNodeStorage_.data();
    workspace.logicNodeStorage_ = std::move(logicNodeStorage_);
    assert(workspace.logicNodeStorage_.data() == logicStorageBase);

    workspace.logicNodes_ = std::move(logicNodes_);

    for (auto idx = size_t{ 0 }, count = static_cast<size_t>(logicNodeCount_); idx < count; idx++) {
        auto&& ln = workspace.logicNodes_[idx];

        workspace.idToLogicNodeIndex_.emplace(ln.get().id(), idx);
        workspace.nameToLogicNodeIndex_.emplace(ln.get().name(), idx);
    }

    workspace.graphicsNodeCount_ = graphicNodeCount_;

    [[maybe_unused]] auto const* graphicsStorageBase = graphicNodeStorage_.data();
    workspace.graphicsNodeStorage_ = std::move(graphicNodeStorage_);
    assert(workspace.graphicsNodeStorage_.data() == graphicsStorageBase);

    workspace.graphicsNodes_ = std::move(graphicNodes_);

    auto surfaceNodeIdx = std::numeric_limits<size_t>::max();
    for (auto idx = size_t{ 0 }, count = static_cast<size_t>(graphicNodeCount_); idx < count; idx++) {
        auto&& gn = workspace.graphicsNodes_[idx];

        workspace.idToGraphicsNodeIndex_.emplace(gn.get().id(), idx);
        workspace.nameToGraphicsNodeIndex_.emplace(gn.get().name(), idx);

        if (gn.isSurfaceNode()) {
            surfaceNodeIdx = idx;
        }
    }

    if (surfaceNodeIdx < static_cast<size_t>(graphicNodeCount_)) {
        auto&& rt = workspace.graphicsNodes_[surfaceNodeIdx].get().getRenderTargetBase();
        auto necessaryFenceCount = rt.frameBufferCount();

        auto& fences = workspace.frameFences_;
        fences.reserve(necessaryFenceCount);

        auto fenceCreateInfo = VkFenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        while (necessaryFenceCount-- > 0) {
            if (auto result = vkCreateFence(device_->handle(),
                                            &fenceCreateInfo,
                                            nullptr,
                                            &fences.emplace_back(device_->handle(), vkDestroyFence));
                result != VK_SUCCESS) {
                throw std::runtime_error("can not create frame sync fence");
            }
        }
    }

    workspace.submits_.resize(static_cast<size_t>(graphicNodeCount_), VkSubmitInfo{});
    workspace.gfxFutures_.resize(static_cast<size_t>(graphicNodeCount_));

    return workspace;
}
}
