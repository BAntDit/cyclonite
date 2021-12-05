//
// Created by bantdit on 3/21/20.
//

#ifndef CYCLONITE_RENDERSYSTEM_H
#define CYCLONITE_RENDERSYSTEM_H

#include "multithreading/taskManager.h"
#include "updateStages.h"
#include "vulkan/device.h"
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

private:
    multithreading::TaskManager* taskManager_;
    vulkan::Device* device_;

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
}

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void RenderSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    if constexpr (STAGE == value_cast(UpdateStage::RENDERING)) {
    }

    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_RENDERSYSTEM_H
