//
// Created by anton on 12/19/20.
//

#ifndef CYCLONITE_WORKSPACE_H
#define CYCLONITE_WORKSPACE_H

#include "graphicsNode.h"
#include "graphicsNodeInterface.h"
#include "logicNode.h"
#include "logicNodeInterface.h"
#include "nodeTypeRegister.h"

namespace cyclonite::compositor {
template<NodeConfig Config>
using node_builder_t = typename std::conditional<config_traits::is_logic_node_v<Config>,
                                                 BaseLogicNode::Builder<Config>,
                                                 BaseGraphicsNode::Builder<Config>>;

class Workspace
{
public:
    Workspace(Workspace&&) = default;

    auto operator=(Workspace &&) -> Workspace& = default;

    ~Workspace();

    void render(vulkan::Device& device);

    [[nodiscard]] auto get(std::string_view name) const -> Node const&;
    auto get(std::string_view name) -> Node&;

    [[nodiscard]] auto get(uint64_t id) const -> Node const&;
    auto get(uint64_t id) -> Node&;

public:
    // TODO::
    class Builder
    {
    public:
        explicit Builder(resources::ResourceManager& resourceManager,
                         vulkan::Device& device,
                         size_t maxBytesPerNode = 64 * 1024,
                         uint8_t maxLogicNodeCount = 10,
                         uint8_t maxGraphicNodeCount = 10);

        template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
        auto createNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if<
          std::is_same_v<node_t<Config>, std::decay_t<std::result_of<NodeFactory(node_builder_t<Config>&&)>>>,
          Builder&>;

        auto build() -> Workspace;

        auto logicNodeNameToId(std::string_view name) -> uint64_t;

    private:
        template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
        auto createLogicNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
          config_traits::is_logic_node_v<Config> &&
            std::is_same_v<LogicNode<Config>,
                           std::decay_t<std::result_of_t<NodeFactory(BaseLogicNode::Builder<Config>&&)>>>,
          Builder&>;

        template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
        auto createGraphicsNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if<
          config_traits::is_graphics_node_v<Config> &&
            std::is_same_v<GraphicsNode<Config>,
                           std::decay_t<std::result_of<NodeFactory(BaseGraphicsNode::Builder<Config>&&)>>>,
          Builder&>;

        [[nodiscard]] auto nodeCount(bool isLogic) const -> uint8_t;
        [[nodiscard]] auto nodeOffset(bool isLogic, size_t index) const -> size_t;
        [[nodiscard]] auto nodeSize(bool isLogic, size_t index) const -> size_t;
        [[nodiscard]] auto nodeStorage(bool isLogic) const -> std::byte const*;

        auto nodeCount(bool isLogic) -> uint8_t&;
        auto nodeOffset(bool isLogic, size_t index) -> size_t&;
        auto nodeSize(bool isLogic, size_t index) -> size_t&;
        auto nodeStorage(bool isLogic) -> std::byte*;

        void emplaceNodeTypeId(uint64_t value);

        template<NodeConfig Config>
        auto allocateNodeMemory()
          -> std::enable_if_t<(sizeof(node_t<Config>) % hardware_constructive_interference_size) == 0, void*>;

        resources::ResourceManager* resourceManager_;
        vulkan::Device* device_;
        std::vector<size_t> logicNodeOffsets_;
        std::vector<size_t> graphicNodeOffsets_;
        std::vector<size_t> logicNodeSizes_;
        std::vector<size_t> graphicNodeSizes_;
        std::vector<std::byte> logicNodeStorage_; // TODO:: change to the aligned storage
        std::vector<std::byte> graphicNodeStorage_;

        std::vector<LogicNodeInterface> logicNodes_;
        std::vector<GraphicsNodeInterface> graphicNodes_;

        uint8_t logicNodeCount_;
        uint8_t graphicNodeCount_;

        std::vector<uint8_t> waitSemaphoresPerNodeCount_;
        std::vector<vulkan::Handle<VkSemaphore>> nodeSignalSemaphores_;
        std::vector<VkSemaphore> nodeWaitSemaphores_;
    };

private:
    Workspace() noexcept;

    void beginFrame();

    void endFrame(vulkan::Device& device);

private:
    uint8_t logicNodeCount_;
    std::vector<std::byte> logicNodeStorage_;
    std::vector<LogicNodeInterface> logicNodes_;
    std::unordered_map<uint64_t, size_t> idToLogicNodeIndex_;
    std::unordered_map<std::string, size_t> nameToLogicNodeIndex_;

    uint8_t graphicsNodeCount_;
    std::vector<std::byte> graphicsNodeStorage_;
    std::vector<GraphicsNodeInterface> graphicsNodes_;
    std::unordered_map<uint64_t, size_t> idToGraphicsNodeIndex_;
    std::unordered_map<std::string, size_t> nameToGraphicsNodeIndex_;

    uint32_t frameNumber_;
    uint32_t frameIndex_;
    uint32_t swapChainLength_;

    std::vector<VkPipelineStageFlags> nodeDstStageMasks_;
    std::vector<VkSemaphore> nodeWaitSemaphores_;
    std::vector<VkSemaphore> nodeSignalSemaphores_;
    std::vector<VkSubmitInfo> submits_;

    uint32_t submitCount_;
    uint8_t presentationNodeIndex_;

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTimeUpdate_;
};

template<NodeConfig Config>
auto Workspace::Builder::allocateNodeMemory()
  -> std::enable_if_t<(sizeof(node_t<Config>) % hardware_constructive_interference_size) == 0, void*>
{
    auto isLogic = config_traits::is_logic_node_v<Config>;

    auto offset = (nodeCount(isLogic) > 0)
                    ? (nodeOffset(isLogic, nodeCount(isLogic) - 1) + nodeSize(isLogic, nodeCount(isLogic) - 1))
                    : size_t{ 0 };

    nodeSize(isLogic, nodeCount(isLogic)) = sizeof(node_t<Config>);
    nodeOffset(isLogic, nodeCount(isLogic)) = offset;

    nodeCount(isLogic)++;

    return nodeStorage(isLogic) + offset;
}

template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
auto Workspace::Builder::createLogicNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
  config_traits::is_logic_node_v<Config> &&
    std::is_same_v<LogicNode<Config>, std::decay_t<std::result_of_t<NodeFactory(BaseLogicNode::Builder<Config>&&)>>>,
  Builder&>
{
    logicNodes_.emplace_back(LogicNodeInterface{
      new (allocateNodeMemory<Config>()) LogicNode<Config>{ nodeFactory(BaseLogicNode::Builder<Config>{
        *resourceManager_, this, &Workspace::Builder::logicNodeNameToId, NodeTypeId::value }) },
      [](void* node, uint32_t frameIndex, real deltaTime) -> void {
          (reinterpret_cast<LogicNode<Config>*>(node))->update(frameIndex, deltaTime);
      },
      [](void* node) -> void { (reinterpret_cast<LogicNode<Config>*>(node))->dispose(); } });

    return *this;
}

template<typename NodeConfig, typename NodeTypeId, typename NodeFactory>
auto Workspace::Builder::createNode(type_pair<Node<NodeConfig>, NodeTypeId>, NodeFactory&& nodeFactory)
  -> std::enable_if_t<
    std::is_same_v<Node<NodeConfig>, std::decay_t<std::result_of_t<NodeFactory(BaseNode::Builder<NodeConfig>&&)>>>,
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
}

#endif // CYCLONITE_WORKSPACE_H
