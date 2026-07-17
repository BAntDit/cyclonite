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
class QueueSubmissionManager;

class QueueSubmission : public core::ResourceBase
{
public:
    QueueSubmission(core::ResourceManagerBase* resourceManager,
                    core::ResourceId resourceId,
                    core::ResourceSharedRef deviceRef,
                    QueueSubmissionManager* manager,
                    uint32_t queueFamilyIndex,
                    CommandPoolFlagBits commandPoolFlags);

    ~QueueSubmission();

    void beginRecording();

    void endRecording();

    void beginBatchRecording(std::string_view batchName);
    void endBatchRecording();

    void addBatchDependency(size_t fromBatch, PipelineStageFlagBits stageMask);

    void addBatchDependency(gfx::SubmissionBatchDependency const& externalDependency);

    void addPresentationSignal(core::ResourceSharedRef const& signal);

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
        std::vector<gfx::SubmissionBatchDependency> dependencies;
        std::vector<gfx::CommandList> commandLists;
        core::ResourceSharedRef signal;
        core::ResourceSharedRef presentationSignal;
    };

    std::unordered_map<std::string, size_t> batchNameToIndex_;
    QueueSubmissionManager* manager_;
    core::ResourceSharedRef commandPool_;
    std::vector<SubmissionBatch> batches_;
    uint64_t completionFrameIndex_;    // to wait for (on CPU for submission completion)
    uint64_t currentFrameIndex_;       // to signal on GPU (for batch signals)
    uint64_t lastCompletedFrameIndex_; // to set as initial value of signals
    QueueSubmissionStateFlagBits state_;

    // TODO:: replace vector with own container
    // that could not implicitly invalidate pointers and iterators from
    std::vector<VkSubmitInfo> vkSubmissions_;
    std::vector<VkTimelineSemaphoreSubmitInfo> vkTimelineSubmissions_;
    std::vector<uint64_t> timelineSemaphoreValues_;
    std::vector<uint64_t> signalValues_;
    std::vector<VkSemaphore> waitSemaphores_;
    std::vector<VkPipelineStageFlags> waitStages_;
    std::vector<VkCommandBuffer> vkCommandBuffers_;
    std::vector<VkSemaphore> vkSignals_;
    size_t dependencyCount_;
    size_t commandBufferCount_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_QUEUE_SUBMISSION_H