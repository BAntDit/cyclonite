//
// Created by bantdit on 11/20/22.
//

#ifndef CYCLONITE_GRAPHICSNODE_H
#define CYCLONITE_GRAPHICSNODE_H

#include "baseGraphicsNode.h"
#include "scene.h"
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

template<NodeConfig Config>
class alignas(hardware_constructive_interference_size) GraphicsNode : public BaseGraphicsNode
{
public:
    using component_config_t = typename Config::component_config_t;
    using systems_config_t = typename Config::systems_config_t;
    using entity_manager_t = enttx::EntityManager<component_config_t>;
    using system_manager_t = enttx::SystemManager<systems_config_t>;
    using scene_t = Scene<component_config_t>;

    explicit GraphicsNode(uint8_t bufferCount);

    auto begin([[maybe_unused]] vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t>;

    [[nodiscard]] auto waitSemaphores() const -> VkSemaphore const* { return nodeWaitSemaphores_.data(); }
    auto waitSemaphores() -> VkSemaphore* { return nodeWaitSemaphores_.data(); }

    [[nodiscard]] auto waitStages() const -> VkPipelineStageFlags const* { return nodeDstStageMasks_.data(); }
    auto waitStages() -> VkPipelineStageFlags* { return nodeDstStageMasks_.data(); }

    void makeExpired(size_t bufferIndex);

    void update(uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime);

    void end(uint32_t waitSemaphoreCount);

private:
    std::array<VkPipelineStageFlags, config_traits::max_wait_semaphore_count_v<Config>> nodeDstStageMasks_;
    std::array<VkSemaphore, config_traits::max_wait_semaphore_count_v<Config>> nodeWaitSemaphores_;
    std::bitset<8> expirationBits_; // 8 - place here max swapchain bits
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
};

template<NodeConfig Config>
auto GraphicsNode<Config>::begin([[maybe_unused]] vulkan::Device& device, uint64_t frameNumber)
  -> std::pair<VkSemaphore, size_t>
{
    auto waitSemaphore = VkSemaphore{ VK_NULL_HANDLE };
    auto commandIndex = size_t{ 0 };

    frameIndex_ = static_cast<uint32_t>(frameNumber % swapChainLength());

    if constexpr (config_traits::is_surface_node_v<Config>) {
        auto& rt = getRenderTarget<SurfaceRenderTarget>();

        auto acquireBackBufferTask = [&rt, &device, frameIndex = frameIndex_]() -> std::pair<VkSemaphore, size_t> {
            auto [imgIndex, wait] = rt.acquireBackBufferIndex(device, frameIndex);
            return std::make_pair(wait, static_cast<size_t>(imgIndex));
        };

        // it has to be fine even in the worker thread,
        // but let it be in the render just in case
        if (multithreading::Render::isInRenderThread()) {
            auto [w, i] = acquireBackBufferTask();
            commandIndex = i;
            waitSemaphore = w;
        } else {
            assert(multithreading::Worker::isInWorkerThread());
            auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(acquireBackBufferTask);
            auto [w, i] = future.get();
            commandIndex = i;
            waitSemaphore = w;
        }
    } else {
        auto& rt = getRenderTarget<FrameBufferRenderTarget>();
        waitSemaphore = rt.wait();
    }

    bufferIndex_ = commandIndex;

    return std::make_pair(waitSemaphore, commandIndex);
}

template<NodeConfig Config>
void GraphicsNode<Config>::makeExpired(size_t bufferIndex)
{
    assert(bufferIndex < expirationBits_.size());
    expirationBits_.set(bufferIndex, false);
}

template<NodeConfig Config>
void GraphicsNode<Config>::update(uint32_t& semaphoreCount, uint64_t frameNumber, real deltaTime)
{
    auto& s = resourceManager().get(scene()).template as<scene_t>();
    systems_.update(s.entities(), *this, semaphoreCount, frameNumber, deltaTime);
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
}

#endif // CYCLONITE_GRAPHICSNODE_H
