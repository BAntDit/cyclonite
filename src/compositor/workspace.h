//
// Created by anton on 12/19/20.
//

#ifndef CYCLONITE_WORKSPACE_H
#define CYCLONITE_WORKSPACE_H

#include "node.h"
#include "nodeInterface.h"

namespace cyclonite::compositor {
class Workspace
{
public:
    Workspace(Workspace&&) = default;

    auto operator=(Workspace &&) -> Workspace& = default;

    ~Workspace();

    void render(vulkan::Device& device);

public:
    class Builder
    {
    public:
        explicit Builder(vulkan::Device& device, uint8_t maxNodeCount = 10, size_t maxBytesPerNode = 64 * 1024);

        template<typename NodeConfig, typename NodeFactory>
        auto createNode(NodeFactory&& nodeFactory) -> std::enable_if_t<
          // std::is_invocable_v<decltype(nodeFactory), cyclonite::compositor::BaseNode::Builder<NodeConfig>&&> &&
          std::is_same_v<Node<NodeConfig>, std::decay_t<std::result_of_t<NodeFactory(vulkan::Device&)>>>,
          Builder&>;

        auto build() -> Workspace;

    private:
        auto allocateNode(size_t size) -> void*;

        vulkan::Device* device_;
        std::vector<uint8_t> waitSemaphoresPerNodeCount_;
        std::vector<vulkan::Handle<VkSemaphore>> nodeSignalSemaphores_;
        std::vector<VkSemaphore> nodeWaitSemaphores_;
        uint8_t nodeCount_;
        std::vector<std::byte> nodeStorage_;
        std::vector<size_t> nodeSizes_;
        std::vector<size_t> nodeOffsets_;
        std::vector<NodeInterface> nodes_;
    };

private:
    Workspace() noexcept;

private:
    uint32_t frameNumber_;
    uint32_t frameIndex_;
    uint32_t swapChainLength_;

    uint8_t nodeCount_;
    std::vector<std::byte> nodeStorage_;
    std::vector<NodeInterface> nodes_;

    std::vector<VkPipelineStageFlags> nodeDstStageMasks_;
    std::vector<VkSemaphore> nodeWaitSemaphores_;
    std::vector<VkSemaphore> nodeSignalSemaphores_;
    std::vector<VkSubmitInfo> submits_;
};

template<typename NodeConfig, typename NodeFactory>
auto Workspace::Builder::createNode(NodeFactory&& nodeFactory) -> std::enable_if_t<
  // std::is_invocable_v<decltype(nodeFactory), cyclonite::compositor::BaseNode::Builder<NodeConfig>&&> &&
  std::is_same_v<Node<NodeConfig>, std::decay_t<std::result_of_t<NodeFactory(vulkan::Device&)>>>,
  Builder&>
{
    nodes_.emplace_back(
      new (allocateNode(sizeof(Node<NodeConfig>)))
        Node{ nodeFactory(cyclonite::compositor::BaseNode::Builder<NodeConfig>{ *device_ }) },
      [](void* node, size_t index) -> void { (reinterpret_cast<Node<NodeConfig>*>(node))->makeExpired(index); },
      []() -> uint32_t { return Node<NodeConfig>::getExpectedWaitSignalCount(); },
      []() -> bool { return Node<NodeConfig>::isSurfaceNode(); },
      [](void* node, vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t> {
          return (reinterpret_cast<Node<NodeConfig>*>(node))->begin(device, frameNumber);
      },
      [](void* node, uint32_t& signalCount, VkSemaphore* baseSignal, VkPipelineStageFlags* baseFlag) -> void {
          (reinterpret_cast<Node<NodeConfig>*>(node))->update(signalCount, baseSignal, baseFlag);
      },
      [](void* node, VkSubmitInfo& submitInfo) -> void {
          (reinterpret_cast<Node<NodeConfig>*>(node))->end(submitInfo);
      },
      [](void* node, vulkan::Device& device) -> FrameCommands& {
          return (reinterpret_cast<Node<NodeConfig>*>(node))->getCurrentFrame(device);
      },
      [](void* node) -> void { (reinterpret_cast<Node<NodeConfig>*>(node))->~Node<NodeConfig>(); });

    return *this;
}
}

#endif // CYCLONITE_WORKSPACE_H
