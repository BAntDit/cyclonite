//
// Created by anton on 10/7/26.
//

#ifndef CYCLONITE_SYSTEMS_TRANSFER_SYSTEM_H
#define CYCLONITE_SYSTEMS_TRANSFER_SYSTEM_H

#include "core/resourceSharedRef.h"
#include "multithreading/dynamicMpscQueue.h"
#include "stages.h"
#include <cstddef>
#include <future>
#include <memory>
#include <metrix/enum.h>
#include <multithreading/executor.h>
#include <multithreading/taskManager.h>
#include <variant>

namespace cyclonite {
template<typename Config>
class Root;
}

namespace cyclonite::systems {
class TransferSystem
{
public:
    enum class TransferTaskType : uint8_t
    {
        BufferTransfer = 0
    };

    struct TransferProps
    {
        core::ResourceSharedRef stagingRef;
        core::ResourceSharedRef gpuResourceRef;
    };

    struct BufferTransferProps : public TransferProps
    {
        size_t srcOffset;
        size_t dstOffset;
        size_t size;
    };

    struct TransferTask
    {
        explicit TransferTask(BufferTransferProps const& props)
          : type(TransferTaskType::BufferTransfer)
          , props(props)
        {
        }

        TransferTaskType type;
        std::variant<BufferTransferProps> props;
    };

    TransferSystem() = default;

    void init(core::ResourceSharedRef const& deviceRef);

    void commitTransferTask(core::ResourceSharedRef const& stagingRef,
                            core::ResourceSharedRef const& gpuResourceRef,
                            size_t srcOffset,
                            size_t dstOffset,
                            size_t size);

    template<size_t ExecutionStage, typename Config>
    auto run(Root<Config>& root,
             std::shared_future<void>& prevStageFutures,
             core::ResourceSharedRef const& sceneRef) -> std::future<void>;

private:
    void transferPrerecord();

    void transferCompletion();

    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef transferSubmissionRef_;
    std::unique_ptr<multithreading::DynamicMpscQueue<TransferTask>> transferTasks_;
};

template<size_t ExecutionStage, typename Config>
auto TransferSystem::run(Root<Config>& root,
                         std::shared_future<void>& prevStageFutures,
                         core::ResourceSharedRef const& sceneRef) -> std::future<void>
{
    if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::TRANSFER_START)) {
        (void)root;
        (void)sceneRef;

        if (prevStageFutures.valid()) {
            prevStageFutures.get();
        }

        auto& taskManager = multithreading::Executor::threadExecutor().taskManager();
        return taskManager.submitTask([this]() -> void { transferPrerecord(); });
    } else if constexpr (ExecutionStage == metrix::value_cast(SystemUpdateStageList::TRANSFER_END)) {
        (void)root;
        (void)sceneRef;

        if (prevStageFutures.valid()) {
            prevStageFutures.get();
        }

        auto& taskManager = multithreading::Executor::threadExecutor().taskManager();
        return taskManager.submitTask([this]() -> void { transferCompletion(); });
    } else {
        auto promise = std::promise<void>();
        auto future = promise.get_future();

        promise.set_value();

        return future;
    }
}
}

#endif // CYCLONITE_SYSTEMS_TRANSFER_SYSTEM_H