//
// Created by anton on 3/21/21.
//

#ifndef CYCLONITE_NODE_H
#define CYCLONITE_NODE_H

#include "baseNode.h"

namespace cyclonite::compositor {
extern void createPass(vulkan::Device& device,
                       uint32_t subPassIndex,
                       bool depthStencilRequired,
                       VkRenderPass renderPass,
                       std::array<uint32_t, 4> const& viewport,
                       uint32_t commandBufferCount,
                       uint32_t imageInputCount,
                       uint32_t attachmentCount,
                       PassType inPassType,
                       PassType& outPassType,
                       vulkan::Handle<VkDescriptorPool>& outDescriptorPool,
                       vulkan::Handle<VkDescriptorSetLayout>& outDescriptorSetLayout,
                       vulkan::Handle<VkPipelineLayout>& outPipelineLayout,
                       vulkan::Handle<VkPipeline>& outPipeline);

template<typename Config>
class Node : public BaseNode
{
public:
    using ecs_config_t = typename Config::ecs_config_t;
    using node_component_list_t = typename ecs_config_t::component_list_t;
    using node_storage_list_t = typename ecs_config_t::storage_list_t;
    using node_system_list_t = typename ecs_config_t::system_list_t;

    using entity_manager_t = typename ecs_config_t::entity_manager_t;
    using system_manager_t = typename ecs_config_t::system_manager_t;

    explicit Node(size_t bufferCount);

    Node(Node const&) = delete;

    Node(Node&&) = default;

    auto operator=(Node const&) -> Node& = delete;

    auto operator=(Node &&) -> Node& = default;

    auto getCurrentFrame() -> FrameCommands&;

    void writeFrameCommands(vulkan::Device& device);

    void makeExpired(size_t swapChainIndex);

    auto begin(vulkan::Device& device, uint64_t frameNumber, VkFence frameFence = VK_NULL_HANDLE)
      -> std::pair<VkSemaphore, size_t>;

    void update(uint32_t& signalCount, VkSemaphore* baseSignal, VkPipelineStageFlags* baseFlag);

    auto end(VkSemaphore* waitSemaphores,
             VkPipelineStageFlags const* waitDstStageMasks,
             uint32_t waitSemaphoreCount) const -> VkSubmitInfo;

    auto systems() -> system_manager_t& { return systems_; }

    [[nodiscard]] auto systems() const -> system_manager_t const& { return systems_; }

    auto entities() -> entity_manager_t& { return entities_; }

    [[nodiscard]] auto entities() const -> entity_manager_t const& { return entities_; }

    ~Node() = default;

    void _createPass(uint32_t subPassIndex,
                     PassType passType,
                     vulkan::Device& device,
                     bool depthStencilRequired,
                     uint32_t imageInputCount);

    static auto isSurfaceNode() -> bool;

    static auto getExpectedWaitSignalCount() -> uint32_t;

private:
    constexpr static bool is_surface_node = Config::is_surface_node;

    constexpr static uint8_t pass_count_v = Config::pass_count;

    entity_manager_t entities_;
    system_manager_t systems_;

    std::array<PassType, pass_count_v> passTypes_;
    std::array<vulkan::Handle<VkDescriptorPool>, pass_count_v> passDescriptorPool_;
    std::array<vulkan::Handle<VkDescriptorSetLayout>, pass_count_v> passDescriptorSetLayout_;
    std::array<vulkan::Handle<VkPipelineLayout>, pass_count_v> passPipelineLayout_;
    std::array<vulkan::Handle<VkPipeline>, pass_count_v> passPipeline_;
    std::array<VkDescriptorSet, pass_count_v> descriptorSets_;
    std::array<std::byte, 4> expirationBits_; // 32 / 8 - put there max possible swap chain length somehow
};

template<typename Config>
Node<Config>::Node(size_t bufferCount)
  : BaseNode{ bufferCount }
  , entities_{}
  , systems_{ &entities_ }
{}

template<typename Config>
auto Node<Config>::getCurrentFrame() -> FrameCommands&
{
    assert(commandsIndex_ < frameCommands_.size());
    return frameCommands_[commandsIndex_];
}

template<typename Config>
void Node<Config>::writeFrameCommands(vulkan::Device& device)
{
    assert(commandsIndex_ < frameCommands_.size());

    auto& frameCommand = frameCommands_[commandsIndex_];

    auto begin = PassIterator{ pass_count_v,
                               0,
                               passTypes_.data(),
                               passDescriptorPool_.data(),
                               passDescriptorSetLayout_.data(),
                               passPipelineLayout_.data(),
                               passPipeline_.data(),
                               descriptorSets_.data(),
                               expirationBits_.data() };

    auto end = PassIterator{ pass_count_v,
                             pass_count_v,
                             passTypes_.data(),
                             passDescriptorPool_.data(),
                             passDescriptorSetLayout_.data(),
                             passPipelineLayout_.data(),
                             passPipeline_.data(),
                             descriptorSets_.data(),
                             expirationBits_.data() };

    auto frameUpdateTask = [&frameCommand,
                            &device,
                            &renderTarget = getRenderTargetBase(),
                            renderPass = static_cast<VkRenderPass>(vkRenderPass_),
                            &inputs = inputs_,
                            &begin,
                            &end]() -> void {
        frameCommand.update(device, renderTarget, renderPass, inputs, begin, end);
    };

    if (multithreading::Render::isInRenderThread()) {
        frameUpdateTask();
    } else {
        assert(multithreading::Worker::isInWorkerThread());
        auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(frameUpdateTask);
        future.get();
    }
}

template<typename Config>
void Node<Config>::makeExpired(size_t swapChainIndex)
{
    assert((swapChainIndex / CHAR_BIT) < expirationBits_.size());

    auto mask = static_cast<std::byte>(1 << (swapChainIndex % CHAR_BIT));
    auto& flags = *(expirationBits_.data() + (swapChainIndex / CHAR_BIT));

    flags |= mask;
}

template<typename Config>
auto Node<Config>::getExpectedWaitSignalCount() -> uint32_t
{
    return static_cast<uint32_t>(node_system_list_t::size);
}

template<typename Config>
auto Node<Config>::begin(vulkan::Device& device, uint64_t frameNumber, VkFence frameFence /* = VK_NULL_HANDLE*/)
  -> std::pair<VkSemaphore, size_t>
{
    (void)frameFence;

    VkSemaphore waitSemaphore = VK_NULL_HANDLE;

    if constexpr (!is_surface_node) {
        // TODO:: make possible to have any number of command buffer for nodes
        // id node commands number != dependency number && command number != 1
        // then copy dependency resource
        commandsIndex_ = 0; // just for now
    }

    if constexpr (is_surface_node) {
        auto&& rt = getRenderTarget<SurfaceRenderTarget>();
        auto frameIndex = frameNumber % rt.frameBufferCount();

        // it acquires image fixedPartIndex for the frame
        // image fixedPartIndex matches with commands for this image
        // (so, fame image fixedPartIndex == commands fixedPartIndex we're going to render)
        // returns: commands fixedPartIndex (image fixedPartIndex), semaphore to be sure image is available
        auto [commandsIndex, wait] = rt.acquireBackBufferIndex(device, frameIndex);

        /*if (fences_[commandsIndex] != VK_NULL_HANDLE) {
            // wait until command buffer for acquired image get free
            vkWaitForFences(device.handle(), 1, &fences_[commandsIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        }

        fences_[commandsIndex] = frameFence;*/

        commandsIndex_ = commandsIndex;

        waitSemaphore = wait;
    }

    return std::make_pair(waitSemaphore, commandsIndex_);
}

template<typename Config>
auto Node<Config>::end(VkSemaphore* waitSemaphores,
                       VkPipelineStageFlags const* waitDstStageMasks,
                       uint32_t waitSemaphoreCount) const -> VkSubmitInfo
{
    auto submit = VkSubmitInfo{};
    auto [count, commands] = frameCommands();
    auto const signalSemaphoreCount = uint32_t{ 1 };

    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.waitSemaphoreCount = waitSemaphoreCount;
    submit.pWaitSemaphores = waitSemaphores;
    submit.pWaitDstStageMask = waitDstStageMasks;
    submit.commandBufferCount = count;
    submit.pCommandBuffers = commands;
    submit.signalSemaphoreCount = signalSemaphoreCount;
    submit.pSignalSemaphores = &passFinishedSemaphore();

    return submit;
}

template<typename Config>
void Node<Config>::update(uint32_t& signalCount, VkSemaphore* baseSignal, VkPipelineStageFlags* baseFlag)
{
    systems_.update(this, std::as_const(*this).cameraEntity(), signalCount, baseSignal, baseFlag);
}

template<typename Config>
void Node<Config>::_createPass(uint32_t subPassIndex,
                               PassType passType,
                               vulkan::Device& device,
                               bool depthStencilRequired,
                               uint32_t imageInputCount)
{
    auto [width, height, bufferCount] = std::visit(
      [](auto&& rt) -> std::tuple<uint32_t, uint32_t, uint32_t> {
          if constexpr (!std::is_same_v<std::decay_t<decltype(rt)>, std::monostate>) {
              return std::make_tuple(rt.width(), rt.height(), rt.frameBufferCount());
          }

          std::terminate();
      },
      renderTarget_);

    std::array<uint32_t, 4> viewport = { 0, 0, width, height };

    createPass(device,
               subPassIndex,
               depthStencilRequired,
               static_cast<VkRenderPass>(vkRenderPass_),
               viewport,
               bufferCount,
               imageInputCount,
               getRenderTargetBase().colorAttachmentCount(),
               passType,
               passTypes_[subPassIndex],
               passDescriptorPool_[subPassIndex],
               passDescriptorSetLayout_[subPassIndex],
               passPipelineLayout_[subPassIndex],
               passPipeline_[subPassIndex]);
}

template<typename Config>
auto Node<Config>::isSurfaceNode() -> bool
{
    return is_surface_node;
}

namespace details {
template<uint64_t Id, typename T, typename... Ts>
struct get_type_id;

template<uint64_t Id, typename T, typename... Ts>
struct get_type_id<Id, T, T, Ts...>
{
    static constexpr uint64_t id_v = Id;
};

template<uint64_t Id, typename T, typename Head, typename... Ts>
struct get_type_id<Id, T, Head, Ts...>
{
    static constexpr uint64_t id_v = get_type_id<Id + 1, T, Ts...>::id_v;
};

template<uint64_t Id, typename T>
struct get_type_id<Id, T>
{
    static constexpr uint64_t id_v = std::numeric_limits<uint64_t>::max();
};

template<uint64_t Id, typename... Ts>
struct get_node_type;

template<typename Head, typename... Ts>
struct get_node_type<0, Head, Ts...>
{
    using type_t = Head;
};

template<uint64_t Id, typename Head, typename... Ts>
struct get_node_type<Id, Head, Ts...>
{
    static_assert(Id < (sizeof...(Ts) + 1));
    using type_t = get_node_type<Id - 1, Ts...>;
};
}

template<typename... NodeTypes>
struct node_type_register
{
    template<typename NodeConfig>
    using node_key_t = easy_mp::type_pair<
      Node<NodeConfig>,
      std::integral_constant<uint64_t, details::get_type_id<0, Node<NodeConfig>, NodeTypes...>::id_v>>;

    template<typename NodeConfig>
    static constexpr auto get_config_type_id() -> uint64_t
    {
        return details::get_type_id<0, Node<NodeConfig>, NodeTypes...>::id_v;
    }

    template<typename NodeType>
    static constexpr auto get_node_type_id() -> uint64_t
    {
        return details::get_type_id<0, NodeType, NodeTypes...>::id_v;
    }

    template<typename NodeConfig>
    using node_type_t = typename details::get_node_type<details::get_type_id<0, Node<NodeConfig>, NodeTypes...>::id_v,
                                                        NodeTypes...>::type_t;
};
}

#endif // CYCLONITE_NODE_H
