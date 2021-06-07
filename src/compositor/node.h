//
// Created by anton on 3/21/21.
//

#ifndef CYCLONITE_NODE_H
#define CYCLONITE_NODE_H

#include "../systems/renderSystem.h"
#include "baseNode.h"

namespace cyclonite::compositor {
template<typename Config>
class Node : public BaseNode
{
public:
    using ecs_config_t = typename Config::ecs_config_t;
    using scene_component_list_t = typename ecs_config_t::scene_component_list_t;
    using scene_storage_list_t = typename ecs_config_t::scene_storage_list_t;
    using scene_system_list_t = typename ecs_config_t::scene_system_list_t;

    using entity_manager_t = typename ecs_config_t::entity_manager_t;
    using system_manager_t = typename ecs_config_t::system_manager_t;

    Node();

    Node(Node const&) = delete;

    Node(Node&&) = default;

    auto operator=(Node const&) -> Node& = delete;

    auto operator=(Node &&) -> Node& = default;

    void begin(vulkan::Device& device, uint32_t frameIndex, VkFence frameFence);

    void render(uint32_t frameIndex, VkFence frameFence) override;

    ~Node() override = default;

    void _createPass(uint32_t subPassIndex,
                     PassType passType,
                     vulkan::Device& device,
                     bool depthStencilRequired,
                     uint32_t imageInputCount);

private:
    constexpr static bool is_surface_node = Config::is_surface_node;

    constexpr static uint8_t pass_count_v = Config::pass_count;

    constexpr static uint8_t dependency_semaphore_count_v = Config::dependecy_node_count;

    constexpr static uint8_t wait_semaphore_count_v =
      is_surface_node ? dependency_semaphore_count_v + 2  // 2 == swap chain semaphore + transfer
                      : dependency_semaphore_count_v + 1; // 1 == transfer semaphore only

    using command_fences_t = std::conditional_t<is_surface_node, std::vector<VkFence>, std::monostate>;

    entity_manager_t entities_;
    system_manager_t systems_;

    command_fences_t fences_;

    std::array<VkSemaphore, wait_semaphore_count_v> waitSemaphores_;
    std::array<VkPipelineStageFlags, wait_semaphore_count_v> waitFlags_;

    std::array<PassType, pass_count_v> passTypes_;
    std::array<vulkan::Handle<VkDescriptorPool>, pass_count_v> passDescriptorPool_;
    std::array<vulkan::Handle<VkDescriptorSetLayout>, pass_count_v> passDescriptorSetLayout_;
    std::array<vulkan::Handle<VkPipelineLayout>, pass_count_v> passPipelineLayout_;
    std::array<vulkan::Handle<VkPipeline>, pass_count_v> passPipeline_;
    std::array<VkDescriptorSet, pass_count_v> descriptorSets_;
    std::array<std::byte, pass_count_v / 8 + 1> expirationBits_;
};

template<typename Config>
Node<Config>::Node()
  : BaseNode()
  , entities_{}
  , systems_{ &entities_ }
{}

template<typename Config>
void Node<Config>::begin(vulkan::Device& device, uint32_t frameIndex, VkFence frameFence)
{
    if constexpr (is_surface_node) {
        auto&& rt = getRenderTarget<SurfaceRenderTarget>();

        // it acquires image index for the frame
        // image index matches with commands for this image
        // (so, fame image index == commands index we going to render)
        // returns: commands index (image index), semaphore to be sure image is available
        auto [commandsIndex, wait] = rt.acquireBackBufferIndex(device, frameIndex);

        if (fences_[commandsIndex] != VK_NULL_HANDLE) {
            // wait until command buffer for acquired image get free
            vkWaitForFences(device.handle(), 1, &fences_[commandsIndex], VK_TRUE, std::numeric_limits<uint64_t>::max());
        }

        fences_[commandsIndex] = frameFence;

        commandsIndex_ = commandsIndex;

        waitSemaphores_[dependency_semaphore_count_v] = wait;
    }
}

template<typename Config>
void Node<Config>::render(uint32_t frameIndex, VkFence frameFence)
{
    systems_->update(this, std::as_const(this)->cameraEntity(), frameIndex, frameFence, .0f);
}

static void createPass(vulkan::Device& device,
                       uint32_t subPassIndex,
                       bool depthStencilRequired,
                       VkRenderPass renderPass,
                       std::array<uint32_t, 4> const& viewport,
                       uint32_t commandBufferCount,
                       uint32_t imageInputCount,
                       PassType inPassType,
                       PassType& outPassType,
                       vulkan::Handle<VkDescriptorPool>& outDescriptorPool,
                       vulkan::Handle<VkDescriptorSetLayout>& outDescriptorSetLayout,
                       vulkan::Handle<VkPipelineLayout>& outPipelineLayout,
                       vulkan::Handle<VkPipeline>& outPipeline);

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
              return std::make_tuple(rt->width(), rt->height(), rt->frameBufferCount());
          }
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
               passType,
               passTypes_[subPassIndex],
               passDescriptorPool_[subPassIndex],
               passDescriptorSetLayout_[subPassIndex],
               passPipelineLayout_[subPassIndex],
               passPipeline_[subPassIndex]);
}

}

#endif // CYCLONITE_NODE_H
