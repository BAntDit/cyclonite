//
// Created by anton on 10/1/25.
//

#ifndef CYCLONITE_VK_QUEUE_SUBMISSION_H
#define CYCLONITE_VK_QUEUE_SUBMISSION_H

#include "core/resourceSharedRef.h"
#include "core/resourceWeakRef.h"
#include "gfx/commandList.h"
#include "gfx/submissionBatchDependency.h"
#include <vector>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class QueueSubmission
{
public:
    [[nodiscard]] addBatch()->uint64_t;

    [[nodiscard]] addCommandList(uint64_t batchId)->gfx::CommandList;

    [[nodiscard]] auto getDependency(uint64_t fromBatchId, PipelineStageFlagBits stageMask)
      -> gfx::SubmissionBatchDependency;

    void addDependency(uint64_t fromBatchId, uint64_t toBatchId, PipelineStageFlagBits stageMask);

    void addDependency(gfx::SubmissionBatchDependency const& dependency);

    void reset();

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
};
}

#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_QUEUE_SUBMISSION_H