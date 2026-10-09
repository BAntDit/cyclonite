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

void TransferSystem::transferPrerecord()
{
    assert(deviceRef_.valid());
    auto& device = deviceRef_.as<gfx::Device>();

    if (transferSubmissionRef_.valid()) {
        auto& prevFrameSubmission = transferSubmissionRef_.as<gfx::QueueSubmission>();

        prevFrameSubmission.waitOnCpu();
        prevFrameSubmission.reset();

        transferSubmissionRef_ = core::ResourceSharedRef{};
    }

    if (!transferTasks_->isEmpty()) {
        auto submissionRef = device.createOneTimeQueueSubmission(multithreading::Purpose::Transfer);
        auto& submission = submissionRef.as<gfx::QueueSubmission>();

        submission.beginRecording();
        submission.beginBatchRecording();
        submission.beginCommandListRecording();

        auto& commandList = submission.commandListToRecord();

        auto commandListUsageFlags = gfx::CommandListUsageFlagBits{ gfx::CommandListUsageFlags::ONE_TIME_SUBMIT };

        commandList.begin(commandListUsageFlags);

        while (auto transferTask = transferTasks_->popFront()) {
            if (transferTask->type == TransferSystem::TransferTaskType::BufferTransfer) {
                auto& transferProps = std::get<TransferSystem::BufferTransferProps>(transferTask->props);
                auto const& stagingRef = transferProps.stagingRef;
                auto& gpuResRef = transferProps.gpuResourceRef;
                auto srcOffset = transferProps.srcOffset;
                auto dstOffset = transferProps.dstOffset;
                auto byteCount = transferProps.size;

                commandList.acquireResourceForTransfer(
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TOP_OF_PIPE_BIT },
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TRANSFER_BIT },
                  gfx::AccessFlagBits{},
                  gfx::AccessFlagBits{ gfx::AccessFlags::TRANSFER_WRITE_BIT },
                  gpuResRef);

                commandList.copyBuffers(stagingRef, gpuResRef, srcOffset, dstOffset, byteCount);

                commandList.releaseResourceToGraphics(
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TRANSFER_BIT },
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::BOTTOM_OF_PIPE_BIT },
                  gfx::AccessFlagBits{ gfx::AccessFlags::TRANSFER_WRITE_BIT },
                  gfx::AccessFlagBits{},
                  gpuResRef);
            }
        }

        commandList.end();
        submission.endBatchRecording();
        submission.endRecording();

        transferSubmissionRef_ = std::move(submissionRef);
    }
}

void TransferSystem::transferCompletion()
{
    if (transferSubmissionRef_.valid()) {
        auto& transferSubmission = transferSubmissionRef_.as<gfx::QueueSubmission>();
        transferSubmission.submit();
    }
}
}
