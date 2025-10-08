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

    [[nodiscard]] auto addBatch() -> uint64_t;

    [[nodiscard]] auto addCommandList(uint64_t batchId) -> gfx::CommandList;

    [[nodiscard]] auto getDependency(uint64_t fromBatchId,
                                     PipelineStageFlagBits stageMask) -> gfx::SubmissionBatchDependency;

    void addDependency(uint64_t fromBatchId, uint64_t toBatchId, PipelineStageFlagBits stageMask);

    void addDependency(uint64_t toBatchId, gfx::SubmissionBatchDependency const& dependency);

    void reset();

    auto waitOnCpu() -> uint64_t;

    // TODO::
    // start recording method (clears submit info) -> recording state

    // after start can add batches, coamnd list and etc.

    // end recording (prepares submit info) -> executable state

    [[nodiscard]] auto purpose() const -> multithreading::Purpose;

    [[nodiscard]] auto signal() const -> core::ResourceSharedRef;

    [[nodiscard]] auto isInInitialState() const -> bool { return state_.test(QueueSubmissionStateFlags::Initial); }

    [[nodiscard]] auto isExecutable() const -> bool
    {
        return state_.value == metrix::value_cast(QueueSubmissionStateFlags::Executable);
    }

    [[nodiscard]] auto isPending() const -> bool { return state_.test(QueueSubmissionStateFlags::Pending); }

    using core::ResourceBase::resourceBase;

private:
    struct SubmissionBatch
    {
        std::vector<gfx::SubmissionBatchDependency> dependencies;
        std::vector<gfx::CommandList> commandLists;
        core::ResourceSharedRef signal;
    };

    core::ResourceSharedRef commandPool_;
    std::vector<SubmissionBatch> batches_;
    uint64_t competitionValue_;
    QueueSubmissionStateFlagBits state_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_QUEUE_SUBMISSION_H