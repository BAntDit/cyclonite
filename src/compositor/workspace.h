//
// Created by anton on 12/19/20.
//

#ifndef CYCLONITE_WORKSPACE_H
#define CYCLONITE_WORKSPACE_H

#include "graphicsNode.h"
#include "graphicsNodeBuilder.h"
#include "graphicsNodeInterface.h"
#include "logicNode.h"
#include "logicNodeBuilder.h"
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

    auto operator=(Workspace&&) -> Workspace& = default;

    ~Workspace();

    void render(vulkan::Device& device);

    [[nodiscard]] auto get(std::string_view name) const -> Node const&;
    auto get(std::string_view name) -> Node&;

    [[nodiscard]] auto get(uint64_t id) const -> Node const&;
    auto get(uint64_t id) -> Node&;

public:
    class Builder
    {
    public:
        Builder(resources::ResourceManager& resourceManager,
                vulkan::Device& device,
                size_t maxBytesPerNode = 64 * 1024,
                uint8_t maxLogicNodeCount = 10,
                uint8_t maxGraphicNodeCount = 10);

        template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
        auto createNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
          std::is_same_v<node_t<Config>, std::decay_t<std::result_of_t<NodeFactory(node_builder_t<Config>&&)>>>,
          Builder&>;

        auto build() -> Workspace;

        [[nodiscard]] auto logicNodeNameToId(std::string_view _name) const -> uint64_t;

        [[nodiscard]] auto graphicsNodeNameToId(std::string_view _name) const -> uint64_t;

        [[nodiscard]] auto nodeNameToId(std::string_view _name) const -> uint64_t;

    private:
        template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
        auto createLogicNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
          config_traits::is_logic_node_v<Config> &&
            std::is_same_v<LogicNode<Config>,
                           std::decay_t<std::result_of_t<NodeFactory(BaseLogicNode::Builder<Config>&&)>>>,
          Builder&>;

        template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
        auto createGraphicsNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
          config_traits::is_graphics_node_v<Config> &&
            std::is_same_v<GraphicsNode<Config>,
                           std::decay_t<std::result_of_t<NodeFactory(BaseGraphicsNode::Builder<Config>&&)>>>,
          Builder&>;

        [[nodiscard]] auto nodeCount(bool isLogic) const -> uint8_t;
        [[nodiscard]] auto nodeOffset(bool isLogic, size_t index) const -> size_t;
        [[nodiscard]] auto nodeSize(bool isLogic, size_t index) const -> size_t;
        [[nodiscard]] auto nodeStorage(bool isLogic) const -> std::byte const*;

        auto nodeCount(bool isLogic) -> uint8_t&;
        auto nodeOffset(bool isLogic, size_t index) -> size_t&;
        auto nodeSize(bool isLogic, size_t index) -> size_t&;
        auto nodeStorage(bool isLogic) -> std::byte*;

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
    };

private:
    Workspace() noexcept;

    void beginFrame();

    void endFrame(vulkan::Device& device, VkFence fence);

    auto syncFrame(vulkan::Device& device) -> VkFence;

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

    uint64_t frameNumber_;

    std::vector<VkSubmitInfo> submits_;
    std::vector<std::shared_future<void>> gfxFutures_;

    uint32_t submitCount_;

    std::vector<vulkan::Handle<VkFence>> frameFences_;

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
auto Workspace::Builder::createNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory) -> std::enable_if_t<
  std::is_same_v<node_t<Config>, std::decay_t<std::result_of_t<NodeFactory(node_builder_t<Config>&&)>>>,
  Builder&>
{
    if constexpr (config_traits::is_logic_node_v<Config>) {
        return createLogicNode(type_pair<Config, NodeTypeId>{}, std::forward<decltype(nodeFactory)>(nodeFactory));
    }

    if constexpr (config_traits::is_graphics_node_v<Config>) {
        return createGraphicsNode(type_pair<Config, NodeTypeId>{}, std::forward<decltype(nodeFactory)>(nodeFactory));
    }

    // can not get here
    std::terminate();
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
      [](void* node, uint64_t frameIndex, real deltaTime) -> void {
          (reinterpret_cast<LogicNode<Config>*>(node))->update(frameIndex, deltaTime);
      },
      [](void* node) -> void { (reinterpret_cast<LogicNode<Config>*>(node))->~LogicNode<Config>(); } });

    return *this;
}

template<NodeConfig Config, typename NodeTypeId, typename NodeFactory>
auto Workspace::Builder::createGraphicsNode(type_pair<Config, NodeTypeId>, NodeFactory&& nodeFactory)
  -> std::enable_if_t<
    config_traits::is_graphics_node_v<Config> &&
      std::is_same_v<GraphicsNode<Config>,
                     std::decay_t<std::result_of_t<NodeFactory(BaseGraphicsNode::Builder<Config>&&)>>>,
    Builder&>
{
    graphicNodes_.emplace_back(GraphicsNodeInterface{
      new (allocateNodeMemory<Config>()) GraphicsNode<Config>{ nodeFactory(BaseGraphicsNode::Builder<Config>{
        *device_, *resourceManager_, this, &Workspace::Builder::nodeNameToId, NodeTypeId::value }) },
      [](void* node) -> void { (reinterpret_cast<GraphicsNode<Config>*>(node))->~GraphicsNode<Config>(); },
      [](void* node, vulkan::Device& device) -> std::pair<VkSemaphore, size_t> {
          return (reinterpret_cast<GraphicsNode<Config>*>(node))->begin(device);
      },
      [](void* node) -> std::pair<VkSemaphore*, VkPipelineStageFlags*> {
          auto* stages = (reinterpret_cast<GraphicsNode<Config>*>(node))->waitStages();
          auto* semaphores = (reinterpret_cast<GraphicsNode<Config>*>(node))->waitSemaphores();

          return std::make_pair(semaphores, stages);
      },
      []() -> bool { return config_traits::is_surface_node_v<Config>; },
      [](void* node) -> void { (reinterpret_cast<GraphicsNode<Config>*>(node))->makeDescriptorSetExpired(); },
      [](void* node, uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime) -> void {
          (reinterpret_cast<GraphicsNode<Config>*>(node))->update(semaphoreCount, frameNumber, deltaTime);
      },
      [](void* node, uint32_t waitSemaphoreCount) -> void {
          (reinterpret_cast<GraphicsNode<Config>*>(node))->end(waitSemaphoreCount);
      },
      [](void* node, vulkan::Device& device) -> void {
          (reinterpret_cast<GraphicsNode<Config>*>(node))->writeFrameCommands(device);
      },
      [](void* node, vulkan::Device& device, uint64_t frameNumber) -> size_t {
          return (reinterpret_cast<GraphicsNode<Config>*>(node))->syncFrame(device, frameNumber);
      } });

    return *this;
}
}

#endif // CYCLONITE_WORKSPACE_H
