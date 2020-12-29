//
// Created by bantdit on 3/21/20.
//

#ifndef CYCLONITE_RENDERSYSTEM_H
#define CYCLONITE_RENDERSYSTEM_H

#include "render/renderPass.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
class RenderSystem : public enttx::BaseSystem<RenderSystem>
{
public:
    RenderSystem() = default;

    RenderSystem(RenderSystem const&) = delete;

    RenderSystem(RenderSystem&&) = default;

    ~RenderSystem() = default;

    auto operator=(RenderSystem const&) -> RenderSystem& = delete;

    auto operator=(RenderSystem &&) -> RenderSystem& = default;

    template<typename... RenderPassArgs>
    void init(multithreading::TaskManager& taskManager, vulkan::Device& device, RenderPassArgs&&... renderPassArgs);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    void finish();

    [[nodiscard]] auto renderPass() const -> render::RenderPass const& { return *renderPass_; }

    [[nodiscard]] auto renderPass() -> render::RenderPass& { return *renderPass_; }

private:
    void _createDummyPipeline(vulkan::Device& device,
                              VkRenderPass renderPass,
                              std::array<uint32_t, 4> const& viewport,
                              bool depthStencilRequired);

    void _createDummyDescriptorPool(vulkan::Device& device, size_t maxSets);

private:
    multithreading::TaskManager* taskManager_;
    vulkan::Device* device_;
    std::unique_ptr<render::RenderPass> renderPass_;

    // tmp // dummy
    vulkan::Handle<VkDescriptorPool> descriptorPool_;
    vulkan::Handle<VkDescriptorSetLayout> descriptorSetLayout_;
    vulkan::Handle<VkPipelineLayout> pipelineLayout_;
    vulkan::Handle<VkPipeline> pipeline_;
};

template<typename... RenderPassArgs>
void RenderSystem::init(multithreading::TaskManager& taskManager,
                        vulkan::Device& device,
                        RenderPassArgs&&... renderPassArgs)
{
    taskManager_ = &taskManager;

    device_ = &device;

    renderPass_ = std::make_unique<render::RenderPass>(device, std::forward<RenderPassArgs>(renderPassArgs)...);

    descriptorPool_ = vulkan::Handle<VkDescriptorPool>{ device.handle(), vkDestroyDescriptorPool };
    descriptorSetLayout_ = vulkan::Handle<VkDescriptorSetLayout>{ device.handle(), vkDestroyDescriptorSetLayout };
    pipelineLayout_ = vulkan::Handle<VkPipelineLayout>{ device.handle(), vkDestroyPipelineLayout };
    pipeline_ = vulkan::Handle<VkPipeline>{ device.handle(), vkDestroyPipeline };

    _createDummyDescriptorPool(*device_, renderPass_->getSwapChainLength());

    _createDummyPipeline(*device_, renderPass_->handle(), renderPass_->viewport(), renderPass_->hasDepthStencil());
}

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void RenderSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);

    if constexpr (STAGE == value_cast(UpdateStage::EARLY_UPDATE)) {
        renderPass_->begin(*device_);
    }

    if constexpr (STAGE == value_cast(UpdateStage::RENDERING)) {
        renderPass_->update(*device_,
                            static_cast<VkDescriptorPool>(descriptorPool_),
                            static_cast<VkDescriptorSetLayout>(descriptorSetLayout_),
                            static_cast<VkPipelineLayout>(pipelineLayout_),
                            static_cast<VkPipeline>(pipeline_));

        auto const& frame = renderPass_->frame();

        auto const& signal = renderPass_->passFinishedSemaphore();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = frame.waitSemaphoreCount();
        submitInfo.pWaitSemaphores = frame.waitSemaphores().data();
        submitInfo.pWaitDstStageMask = frame.waitFlags().data();
        submitInfo.commandBufferCount = frame.graphicsCommandCount();
        submitInfo.pCommandBuffers = frame.graphicsCommands();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signal;

        if (auto result = vkQueueSubmit(device_->graphicsQueue(), 1, &submitInfo, renderPass_->fence());
            result != VK_SUCCESS) {
            throw std::runtime_error("could not submit graphics commands");
        }

        renderPass_->end(*device_);
    }
}
}

#endif // CYCLONITE_RENDERSYSTEM_H
