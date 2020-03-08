//
// Created by bantdit on 2/22/20.
//

#ifndef CYCLONITE_TRANSFERSYSTEM_H
#define CYCLONITE_TRANSFERSYSTEM_H

#include "../vulkan/device.h"
#include "cameraSystem.h"
#include "meshSystem.h"
#include "updateStages.h"
#include <easy-mp/enum.h>
#include <enttx/enttx.h>

namespace cyclonite::systems {
class TransferSystem : public enttx::BaseSystem<TransferSystem>
{
public:
    TransferSystem() = default;

    ~TransferSystem() = default;

    void init(vulkan::Device& device);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

private:
    VkDevice vkDevice_;
    uint32_t transferVersion_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void TransferSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    (void)entityManager;

    if constexpr (STAGE == easy_mp::value_cast(UpdateStage::TRANSFER_STAGE)) {
        auto&& [frameCommands, camera, dt] = std::forward_as_tuple(std::forward<Args>(args)...);
        (void)camera;
        (void)dt;

        if (transferVersion_ != frameCommands.transferVersion()) {
            auto const& cameraSystem = std::as_const(systemManager).template get<CameraSystem>();
            auto& meshSystem = systemManager.template get<MeshSystem>();

            frameCommands.updatePersistentTransfer([&](auto&& transfer, auto&& semaphores, auto&& flags) -> void {
                transfer.push_back(meshSystem.persistentTransferCommands());
                semaphores.push_back(meshSystem.transferSemaphore());
                flags.push_back(VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);

                transfer.push_back(cameraSystem.persistentTransferCommands());
                semaphores.push_back(cameraSystem.transferSemaphore());
                flags.push_back(VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
            });

            if (meshSystem.transientTransferCommands()) {
                frameCommands.updateTransientTransfer([&](auto&& transfer, auto&& semaphores, auto&& flags) -> void {
                    transfer.push_back(std::move(meshSystem.transientTransferCommands()));

                    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
                    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

                    if (auto result = vkCreateSemaphore(vkDevice_,
                                                        &semaphoreCreateInfo,
                                                        nullptr,
                                                        &semaphores.emplace_back(vkDevice_, vkDestroySemaphore));
                        result != VK_SUCCESS) {
                        throw std::runtime_error("could not create vertices transfer semaphore.");
                    }

                    flags.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
                });
            }
        }

        transferVersion_ = frameCommands.transferVersion();
    }
}
}

#endif // CYCLONITE_TRANSFERSYSTEM_H
