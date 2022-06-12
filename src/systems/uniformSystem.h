//
// Created by bantdit on 3/21/20.
//

#ifndef CYCLONITE_UNIFORMSYSTEM_H
#define CYCLONITE_UNIFORMSYSTEM_H

#include "../typedefs.h"
#include "renderSystem.h"
#include "resources/staging.h"
#include "updateStages.h"
#include "vulkan/commandPool.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
class UniformSystem : public enttx::BaseSystem<UniformSystem>
{
public:
    using transfer_commands_t = vulkan::CommandBufferSet<vulkan::CommandPool, std::array<VkCommandBuffer, 1>>;

    UniformSystem() = default;

    UniformSystem(UniformSystem const&) = delete;

    UniformSystem(UniformSystem&&) = default;

    ~UniformSystem() = default;

    auto operator=(UniformSystem const&) -> UniformSystem& = delete;

    auto operator=(UniformSystem &&) -> UniformSystem& = default;

    void init(resources::ResourceManager& resourceManager, vulkan::Device& device, size_t swapChainLength);

    [[nodiscard]] auto uniforms() const -> resources::Staging const&;

    auto uniforms() -> resources::Staging&;

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    // tmp
    void setViewMatrix(mat4& viewMatrix);

    void setProjectionMatrix(mat4& projectionMatrix);

    void setViewProjectionMatrix(mat4& viewProjMatrix);

private:
    vulkan::Device* devicePtr_;
    resources::ResourceManager* resourceManager_;
    VkQueue vkTransferQueue_;
    resources::Resource::Id uniforms_;
    std::shared_ptr<vulkan::Buffer> gpuUniforms_;
    std::vector<vulkan::Handle<VkSemaphore>> transferSemaphores_;
    std::unique_ptr<transfer_commands_t> transferCommands_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void UniformSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    (void)entityManager;
    (void)systemManager;

    ((void)args, ...);

    if constexpr (STAGE == value_cast(UpdateStage::TRANSFER_STAGE)) {
        auto&& [node, cameraEntity, signalCount, baseSignal, baseMask] =
          std::forward_as_tuple(std::forward<Args>(args)...);

        auto& frame = node->getCurrentFrame();
        auto idx = (*node).commandIndex();
        auto const& signal = std::as_const(transferSemaphores_[idx]);

        *(baseSignal + signalCount) = static_cast<VkSemaphore>(signal);
        *(baseMask + signalCount) = VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
        signalCount++;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = transferCommands_->commandBufferCount();
        submitInfo.pCommandBuffers = transferCommands_->pCommandBuffers();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &signal;

        if (auto result = vkQueueSubmit(vkTransferQueue_, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
            throw std::runtime_error("could not transfer uniforms");
        }

        frame.setUniformBuffer(devicePtr_->graphicsQueue(), gpuUniforms_);
    }
}
}

#endif // CYCLONITE_UNIFORMSYSTEM_H
