//
// Created by bantdit on 3/21/20.
//

#ifndef CYCLONITE_UNIFORMSYSTEM_H
#define CYCLONITE_UNIFORMSYSTEM_H

#include "../typedefs.h"
#include "renderSystem.h"
#include "updateStages.h"
#include "vulkan/commandPool.h"
#include "vulkan/staging.h"
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

    void init(vulkan::Device& device);

    [[nodiscard]] auto uniforms() const -> vulkan::Staging const& { return *uniforms_; }

    [[nodiscard]] auto uniforms() -> vulkan::Staging& { return *uniforms_; }

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    // tmp
    void setViewMatrix(mat4& viewMatrix);

    void setProjectionMatrix(mat4& projectionMatrix);

    void setViewProjectionMatrix(mat4& viewProjMatrix);

private:
    VkDevice vkDevice_;
    VkQueue vkTransferQueue_;
    size_t transferSemaphoreId_;
    std::unique_ptr<vulkan::Staging> uniforms_;
    std::shared_ptr<vulkan::Buffer> gpuUniforms_;
    std::unique_ptr<transfer_commands_t> transferCommands_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void UniformSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    ((void)args, ...);

    if constexpr (STAGE == value_cast(UpdateStage::TRANSFER_STAGE)) {
        auto& renderSystem = systemManager.template getSystem<RenderSystem>();
        auto& renderPass = renderSystem.renderPass();
        auto& frame = renderPass.frame();

        auto [id, signal] = frame.getWaitSemaphore(transferSemaphoreId_, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);

        transferSemaphoreId_ = id;

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = transferCommands_->commandBufferCount();
        submitInfo.pCommandBuffers = transferCommands_->pCommandBuffers();
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signal;

        if (auto result = vkQueueSubmit(vkTransferQueue_, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) {
            throw std::runtime_error("could not transfer uniforms");
        }

        frame.setUniformBuffer(gpuUniforms_);
    }
}
}

#endif // CYCLONITE_UNIFORMSYSTEM_H
