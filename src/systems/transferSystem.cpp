//
// Created by anton on 10/7/26.
//

#include "transferSystem.h"
#include "gfx/device.h"
#include "gfx/queueSubmission.h"
#include "gfx/queueSubmissionRecorder.h"
#include <cassert>

namespace cyclonite::systems {
void TransferSystem::init(core::ResourceSharedRef const& deviceRef)
{
    deviceRef_ = deviceRef;
    transferTasks_ = std::make_unique<multithreading::DynamicMpscQueue<TransferTask>>();
}

void TransferSystem::commitTransferTask(core::ResourceSharedRef const& stagingRef,
                                        core::ResourceSharedRef const& gpuResourceRef,
                                        size_t srcOffset,
                                        size_t dstOffset,
                                        size_t size)
{
    assert(transferTasks_);

    auto transferProps = BufferTransferProps{};
    transferProps.stagingRef = stagingRef;
    transferProps.gpuResourceRef = gpuResourceRef;
    transferProps.srcOffset = srcOffset;
    transferProps.dstOffset = dstOffset;
    transferProps.size = size;

    transferTasks_->emplaceBack(transferProps);
}

namespace {
struct TransferJob
{
    void operator()()
    {
        auto& device = deviceRef_.as<gfx::Device>();

        device.createQ

          // add aquire one time independent submission
          auto submissionRef =
          device.queueSubmissionManager().getSubmission(multithreading::Purpose::Transfer,
                                                        gfx::CommandPoolFlagBits{ gfx::CommandPoolFlags::TRANSIENT },
                                                        gfx::default_transfer_submission_priority_v);

        auto& transferSubmission = submissionRef.as<gfx::QueueSubmission>();
        auto isInRecordingState = transferSubmission.isInRecordingState();

        auto transferSubmissionRecorder = gfx::QueueSubmissionRecorder{ submissionRef };

        if (!isInRecordingState) {
            transferSubmissionRecorder.start();
        }

        auto batchRecorder = transferSubmissionRecorder.addBatch();

        auto commandListRecorder = batchRecorder.addCommandList();

        commandListRecorder.begin(gfx::CommandListUsageFlagBits{ gfx::CommandListUsageFlags::ONE_TIME_SUBMIT });

        if (transferTask_.type == TransferSystem::TransferTaskType::BufferTransfer) {
            auto& transferProps = std::get<TransferSystem::BufferTransferProps>(transferTask_.props);
            auto const& stagingRef = transferProps.stagingRef;
            auto const& gpuResRef = transferProps.gpuResourceRef;
            auto srcOffset = transferProps.srcOffset;
            auto dstOffset = transferProps.dstOffset;
            auto byteCount = transferProps.size;

            // TODO:: test if necessary
            // acquiring barrier
            commandListRecorder.acquireResourceForTransfer(
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TOP_OF_PIPE_BIT },
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TRANSFER_BIT },
              gfx::AccessFlagBits{},
              gfx::AccessFlagBits{ gfx::AccessFlags::TRANSFER_WRITE_BIT },
              gpuResRef);

            commandListRecorder.copyBuffers(stagingRef, gpuResRef, srcOffset, dstOffset, byteCount);

            // release barrier
            commandListRecorder.releaseResourceToGraphics(
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TRANSFER_BIT },
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::BOTTOM_OF_PIPE_BIT },
              gfx::AccessFlagBits{ gfx::AccessFlags::TRANSFER_WRITE_BIT },
              gfx::AccessFlagBits{},
              gpuResRef);
        }

        commandListRecorder.end();

        // one batch
    }

    core::ResourceSharedRef deviceRef_;
    TransferSystem::TransferTask transferTask_;
};
}

auto TransferSystem::transfer() -> std::future<void>
{
    while (auto transferTask = transferTasks_->popFront()) {
        // TODO:: transfer job
    }
}
}
