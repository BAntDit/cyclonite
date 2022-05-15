//
// Created by anton on 12/19/20.
//

#ifndef CYCLONITE_WORKSPACE_H
#define CYCLONITE_WORKSPACE_H

#include "node.h"
#include "nodeBuilder.h"
#include "nodeInterface.h"

namespace cyclonite::compositor {
class Workspace
{
public:
    Workspace(Workspace&&) = default;

    auto operator=(Workspace &&) -> Workspace& = default;

    ~Workspace();

    void render(vulkan::Device& device);

    template<typename NodeConfig, typename NodeTypeId>
    auto get(type_pair<Node<NodeConfig>, NodeTypeId>) -> Node<NodeConfig>&;

    template<typename NodeConfig, typename NodeTypeId>
    [[nodiscard]] auto get(type_pair<Node<NodeConfig>, NodeTypeId>) const -> Node<NodeConfig> const&;

public:
    class Builder
    {
    public:
        explicit Builder(vulkan::Device& device, uint8_t maxNodeCount = 10, size_t maxBytesPerNode = 64 * 1024);

        template<typename NodeConfig, typename NodeTypeId, typename NodeFactory>
        auto createNode(type_pair<Node<NodeConfig>, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
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
        std::vector<uint64_t> nodeTypeIds_;
    };

private:
    Workspace() noexcept;

    void beginFrame();

    void endFrame(vulkan::Device& device);

private:
    uint32_t frameNumber_;
    uint32_t frameIndex_;
    uint32_t swapChainLength_;

    uint8_t nodeCount_;
    std::vector<std::byte> nodeStorage_;
    std::vector<NodeInterface> nodes_;
    std::vector<uint64_t> nodeTypeIds_;

    std::vector<VkPipelineStageFlags> nodeDstStageMasks_;
    std::vector<VkSemaphore> nodeWaitSemaphores_;
    std::vector<VkSemaphore> nodeSignalSemaphores_;
    std::vector<VkSubmitInfo> submits_;

    uint32_t submitCount_;
    uint8_t presentationNodeIndex_;
};

template<typename NodeConfig, typename NodeTypeId, typename NodeFactory>
auto Workspace::Builder::createNode(type_pair<Node<NodeConfig>, NodeTypeId>, NodeFactory&& nodeFactory)
  -> std::enable_if_t<std::is_same_v<Node<NodeConfig>, std::decay_t<std::result_of_t<NodeFactory(vulkan::Device&)>>>,
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
      [](void* node,
         VkSemaphore* waitSemaphores,
         VkPipelineStageFlags const* waitDstStageMasks,
         uint32_t waitSemaphoreCount) -> VkSubmitInfo {
          return std::as_const(*reinterpret_cast<Node<NodeConfig>*>(node))
            .end(waitSemaphores, waitDstStageMasks, waitSemaphoreCount);
      },
      [](void* node) -> FrameCommands& { return (reinterpret_cast<Node<NodeConfig>*>(node))->getCurrentFrame(); },
      [](void* node) -> void { (reinterpret_cast<Node<NodeConfig>*>(node))->~Node<NodeConfig>(); },
      [](void* node, vulkan::Device& device) -> void {
          return (reinterpret_cast<Node<NodeConfig>*>(node))->writeFrameCommands(device);
      });

    nodeTypeIds_.emplace_back(NodeTypeId::value);

    return *this;
}

template<typename NodeConfig, typename NodeTypeId>
auto Workspace::get(type_pair<Node<NodeConfig>, NodeTypeId>) -> Node<NodeConfig>&
{
    void* node = nullptr;

    for (auto i = size_t{ 0 }, count = nodeTypeIds_.size(); i < count; i++) {
        auto id = nodeTypeIds_[i];

        if (id == NodeTypeId::value) {
            node = nodes_[i].getRawPtr();
            break;
        }
    }

    assert(node != nullptr);

    return *(reinterpret_cast<Node<NodeConfig>*>(node));
}

template<typename NodeConfig, typename NodeTypeId>
auto Workspace::get(type_pair<Node<NodeConfig>, NodeTypeId>) const -> Node<NodeConfig> const&
{
    void const* node = nullptr;

    for (auto i = size_t{ 0 }, count = nodeTypeIds_.size(); i < count; i++) {
        auto id = nodeTypeIds_[i];

        if (id == NodeTypeId::value) {
            node = nodes_[i].getRawPtr();
            break;
        }
    }

    assert(node != nullptr);

    return *(reinterpret_cast<Node<NodeConfig> const*>(node));
}
}

#endif // CYCLONITE_WORKSPACE_H
