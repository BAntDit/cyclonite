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

    auto operator=(RenderSystem&&) -> RenderSystem& = default;

    void init(multithreading::TaskManager& taskManager, vulkan::Device& device);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    void finish();

private:
    multithreading::TaskManager* taskManager_;
    vulkan::Device* device_;
};

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void RenderSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    using namespace easy_mp;

    if constexpr (STAGE == value_cast(UpdateStage::RENDERING)) {
        auto&& [node, semaphoreCount, frameNumber, dt] = std::forward_as_tuple(std::forward<Args>(args)...);

        (void)semaphoreCount;
        (void)frameNumber;
        (void)dt;

        assert(device_ != nullptr);

        auto writeCommandsTask = [&device = *device_, &node = node]() -> void { node.writeFrameCommands(device); };

        if (multithreading::Render::isInRenderThread()) {
            writeCommandsTask();
        } else {
            assert(multithreading::Worker::isInWorkerThread());
            auto future = multithreading::Worker::threadWorker().taskManager().submitRenderTask(writeCommandsTask);
            future.get();
        }
    }

    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_RENDERSYSTEM_H
