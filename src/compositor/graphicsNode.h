//
// Created by bantdit on 11/20/22.
//

#ifndef CYCLONITE_GRAPHICSNODE_H
#define CYCLONITE_GRAPHICSNODE_H

#include "baseGraphicsNode.h"
#include "nodeAsset.h"
#include "passIterator.h"
#include "typedefs.h"
#include "vulkan/shaderModule.h"

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
                       std::unique_ptr<vulkan::ShaderModule>& vertexSceneShader,
                       std::unique_ptr<vulkan::ShaderModule>& fragmentSceneShader,
                       std::unique_ptr<vulkan::ShaderModule>& vertexScreenShader,
                       std::unique_ptr<vulkan::ShaderModule>& fragmentScreenShader,
                       vulkan::Handle<VkDescriptorPool>& outDescriptorPool,
                       vulkan::Handle<VkDescriptorSetLayout>& outDescriptorSetLayout,
                       vulkan::Handle<VkPipelineLayout>& outPipelineLayout,
                       vulkan::Handle<VkPipeline>& outPipeline);

/**
 * Graphics node class
 */
template<NodeConfig Config>
class alignas(hardware_constructive_interference_size) GraphicsNode : public BaseGraphicsNode
{
public:
    using component_config_t = typename Config::component_config_t;
    using systems_config_t = typename Config::systems_config_t;
    using entity_manager_t = enttx::EntityManager<component_config_t>;
    using system_manager_t = enttx::SystemManager<systems_config_t>;
    using asset_t = NodeAsset<component_config_t>;

    GraphicsNode(resources::ResourceManager& resourceManager,
                 std::string_view name,
                 uint64_t typeId,
                 uint8_t bufferCount = 1);

    GraphicsNode(GraphicsNode&& graphicsNode) = default;

    GraphicsNode(GraphicsNode const&) = delete;

    ~GraphicsNode() = default;

    auto operator=(GraphicsNode&&) -> GraphicsNode& = default;

    auto operator=(GraphicsNode const&) -> GraphicsNode& = delete;

    auto syncFrame(vulkan::Device& device, uint64_t frameNumber) -> size_t;

    auto begin([[maybe_unused]] vulkan::Device& device) -> std::pair<VkSemaphore, size_t>;

    [[nodiscard]] auto waitSemaphores() const -> VkSemaphore const* { return nodeWaitSemaphores_.data(); }
    auto waitSemaphores() -> VkSemaphore* { return nodeWaitSemaphores_.data(); }

    [[nodiscard]] auto waitStages() const -> VkPipelineStageFlags const* { return nodeDstStageMasks_.data(); }
    auto waitStages() -> VkPipelineStageFlags* { return nodeDstStageMasks_.data(); }

    void makeDescriptorSetExpired();

    void writeFrameCommands(vulkan::Device& device);

    void update(uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime);

    void end(uint32_t waitSemaphoreCount);

    [[nodiscard]] auto systems() const -> system_manager_t const& { return systems_; }
    auto systems() -> system_manager_t& { return systems_; }

private:
    friend class BaseGraphicsNode::Builder<Config>;

    void _createPass(uint32_t subPassIndex,
                     PassType passType,
                     vulkan::Device& device,
                     bool depthStencilRequired,
                     uint32_t imageInputCount);

private:
    std::array<VkPipelineStageFlags, config_traits::max_wait_semaphore_count_v<Config>> nodeDstStageMasks_;
    std::array<VkSemaphore, config_traits::max_wait_semaphore_count_v<Config>> nodeWaitSemaphores_;
    system_manager_t systems_;

    // dummy, tmp solution:
    std::unique_ptr<vulkan::ShaderModule> vertexSceneShader_;
    std::unique_ptr<vulkan::ShaderModule> fragmentSceneShader_;
    std::unique_ptr<vulkan::ShaderModule> vertexScreenShader_;
    std::unique_ptr<vulkan::ShaderModule> fragmentScreenShader_;

    std::array<PassType, config_traits::pass_count_v<Config>> passTypes_;

    // material
    std::array<vulkan::Handle<VkDescriptorPool>, config_traits::pass_count_v<Config>> passDescriptorPool_;
    std::array<vulkan::Handle<VkDescriptorSetLayout>, config_traits::pass_count_v<Config>> passDescriptorSetLayout_;
    std::array<vulkan::Handle<VkPipelineLayout>, config_traits::pass_count_v<Config>> passPipelineLayout_;
    std::array<vulkan::Handle<VkPipeline>, config_traits::pass_count_v<Config>> passPipeline_;
    std::array<VkDescriptorSet, config_traits::pass_count_v<Config>> descriptorSets_;
    std::bitset<config_traits::pass_count_v<Config>> descriptorSetExpirationBits_;
};

template<NodeConfig Config>
GraphicsNode<Config>::GraphicsNode(resources::ResourceManager& resourceManager,
                                   std::string_view name,
                                   uint64_t typeId,
                                   uint8_t bufferCount /*= 1*/)
  : BaseGraphicsNode{ resourceManager, name, typeId, bufferCount }
  , nodeDstStageMasks_{}
  , nodeWaitSemaphores_{}
  , systems_{}
  , passTypes_{}
  , passDescriptorPool_{}
  , passDescriptorSetLayout_{}
  , passPipelineLayout_{}
  , passPipeline_{}
  , descriptorSets_{}
  , descriptorSetExpirationBits_{}
{
    descriptorSetExpirationBits_.set();
}

template<NodeConfig Config>
auto GraphicsNode<Config>::syncFrame(vulkan::Device& device, uint64_t frameNumber) -> size_t
{
    auto syncIndex = size_t{ 0 };

    frameIndex_ = static_cast<uint32_t>(frameNumber % swapChainLength());

    if constexpr (config_traits::is_surface_node_v<Config>) {
        auto& rt = getRenderTarget<SurfaceRenderTarget>();
        auto [imgIndex, wait] = rt.acquireBackBufferIndex(device, frameIndex_);
        (void)wait;

        syncIndex = imgIndex;
        bufferIndex_ = imgIndex;
    } else {
        bufferIndex_ = 0;
    }

    return syncIndex;
}

template<NodeConfig Config>
auto GraphicsNode<Config>::begin([[maybe_unused]] vulkan::Device& device) -> std::pair<VkSemaphore, size_t>
{
    assert(multithreading::Render::isInRenderThread());

    auto waitSemaphore = VkSemaphore{ VK_NULL_HANDLE };
    auto commandIndex = bufferIndex_;

    if constexpr (config_traits::is_surface_node_v<Config>) {
        auto& rt = getRenderTarget<SurfaceRenderTarget>();
        waitSemaphore = rt.wait(frameIndex_);
    } else {
        auto& rt = getRenderTarget<FrameBufferRenderTarget>();
        waitSemaphore = rt.wait();

        if (rt.signal() == VK_NULL_HANDLE) {
            rt._createSignal(device); // TODO:: make out somewhere and make possible to run not in the render thread
        }
    }

    return std::make_pair(waitSemaphore, commandIndex);
}

template<NodeConfig Config>
void GraphicsNode<Config>::makeDescriptorSetExpired()
{
    descriptorSetExpirationBits_.set(); // all passes at once
}

template<NodeConfig Config>
void GraphicsNode<Config>::update(uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime)
{
    auto& a = resourceManager().get(asset()).template as<asset_t>();
    systems_.update(a.entities(), *this, semaphoreCount, frameNumber, deltaTime);
}

template<NodeConfig Config>
void GraphicsNode<Config>::end(uint32_t waitSemaphoreCount)
{
    submit_ = VkSubmitInfo{};

    auto [count, commands] = frameCommands();
    auto const signalSemaphoreCount = uint32_t{ 1 };

    submit_.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_.waitSemaphoreCount = waitSemaphoreCount;
    submit_.pWaitSemaphores = waitSemaphores();
    submit_.pWaitDstStageMask = waitStages();
    submit_.commandBufferCount = count;
    submit_.pCommandBuffers = commands;
    submit_.signalSemaphoreCount = signalSemaphoreCount;
    submit_.pSignalSemaphores = passFinishedSemaphorePtr();
}

template<NodeConfig Config>
void GraphicsNode<Config>::writeFrameCommands(vulkan::Device& device)
{
    constexpr auto pass_count_v = config_traits::pass_count_v<Config>;

    auto begin = PassIterator{ pass_count_v,
                               0,
                               passTypes_.data(),
                               passDescriptorPool_.data(),
                               passDescriptorSetLayout_.data(),
                               passPipelineLayout_.data(),
                               passPipeline_.data(),
                               descriptorSets_.data(),
                               descriptorSetExpirationBits_ };

    auto end = PassIterator{ pass_count_v,
                             pass_count_v,
                             passTypes_.data(),
                             passDescriptorPool_.data(),
                             passDescriptorSetLayout_.data(),
                             passPipelineLayout_.data(),
                             passPipeline_.data(),
                             descriptorSets_.data(),
                             descriptorSetExpirationBits_ };

    auto frameUpdateTask = [this,
                            &device,
                            &renderTarget = getRenderTargetBase(),
                            renderPass = static_cast<VkRenderPass>(vkRenderPass_),
                            &inputs = inputs_,
                            &begin,
                            &end]() -> void {
        auto commandIndex = bufferIndex_;

        assert(commandIndex < frameCommands_.size());
        auto& frameCommand = frameCommands_[commandIndex];

        frameCommand.update(device, renderTarget, renderPass, inputs, begin, end);

        descriptorSetExpirationBits_.reset(); // all descriptors must be up-to-date here
    };

    if (multithreading::Render::isInRenderThread()) {
        frameUpdateTask();
    } else {
        assert(multithreading::Worker::isInWorkerThread());
        auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(frameUpdateTask);
        future.get();
    }
}

template<NodeConfig Config>
void GraphicsNode<Config>::_createPass(uint32_t subPassIndex,
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

    auto viewport = std::array<uint32_t, 4>{ 0, 0, width, height };

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
               vertexSceneShader_,
               fragmentSceneShader_,
               vertexScreenShader_,
               fragmentScreenShader_,
               passDescriptorPool_[subPassIndex],
               passDescriptorSetLayout_[subPassIndex],
               passPipelineLayout_[subPassIndex],
               passPipeline_[subPassIndex]);
}
}

#endif // CYCLONITE_GRAPHICSNODE_H
