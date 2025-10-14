//
// Created by anton on 10/1/25.
//

#ifndef CYCLONITE_VK_QUEUE_SUBMISSION_H
#define CYCLONITE_VK_QUEUE_SUBMISSION_H

#include "core/resourceSharedRef.h"
#include "gfx/commandList.h"
#include "gfx/submissionBatchDependency.h"
#include <vector>

#include "multithreading/executor.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class QueueSubmission : public core::ResourceBase
{
public:
    QueueSubmission(core::ResourceManagerBase* resourceManager,
                    core::ResourceId resourceId,
                    core::ResourceSharedRef deviceRef,
                    uint32_t queueFamilyIndex,
                    CommandPoolFlagBits commandPoolFlags);

    void beginRecording();

    void endRecording();

    void beginBatchRecording();
    void endBatchRecording();

    void addBatchDependency(size_t fromBatch, PipelineStageFlagBits stageMask);

    void addBatchDependency(gfx::SubmissionBatchDependency const& externalDependency);

    void beginCommandListRecording();
    void endCommandListRecording();

    void reset();

    auto waitOnCpu() -> uint64_t;

    void submit();

    [[nodiscard]] auto commandListToRecord() -> gfx::CommandList&;

    [[nodiscard]] auto purpose() const -> multithreading::Purpose;

    [[nodiscard]] auto signal() const -> core::ResourceSharedRef;

    [[nodiscard]] auto isInInitialState() const -> bool { return state_.test(QueueSubmissionStateFlags::Initial); }

    [[nodiscard]] auto isInRecordingState() const -> bool { return state_.test(QueueSubmissionStateFlags::Recording); }

    [[nodiscard]] auto isInBatchRecordingState() const -> bool
    {
        return state_.test(QueueSubmissionStateFlags::BatchRecording);
    }

    [[nodiscard]] auto isInCommandListRecordingState() const -> bool
    {
        return state_.test(QueueSubmissionStateFlags::CommandListRecording);
    }

    [[nodiscard]] auto isExecutable() const -> bool
    {
        return state_.value == metrix::value_cast(QueueSubmissionStateFlags::Executable);
    }

    [[nodiscard]] auto isPending() const -> bool { return state_.test(QueueSubmissionStateFlags::Pending); }

    [[nodiscard]] auto currentFrameIndex() const -> uint64_t { return currentFrameIndex_; };

    void setFrameIndices(uint64_t currentFrameIndex, uint64_t lastCompletedFrameIndex);

    using core::ResourceBase::resourceBase;

private:
    struct SubmissionBatch
    {
        std::vector<gfx::SubmissionBatchDependency> timelineDependencies;
        std::vector<gfx::SubmissionBatchDependency> binaryDependencies;
        std::vector<gfx::CommandList> commandLists;
        core::ResourceSharedRef signal;
    };

    core::ResourceSharedRef commandPool_;
    std::vector<SubmissionBatch> batches_;
    uint64_t completionFrameIndex_;    // to wait for (on CPU for submission completion)
    uint64_t currentFrameIndex_;       // to signal on GPU (for batch signals)
    uint64_t lastCompletedFrameIndex_; // to set as initial value of signals
    QueueSubmissionStateFlagBits state_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_QUEUE_SUBMISSION_H