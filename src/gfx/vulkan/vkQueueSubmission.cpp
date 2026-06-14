
#include "vkQueueSubmission.h"
#include "gfx/commandPool.h"
#include "gfx/device.h"
#include "gfx/signal.h"
#include "multithreading/taskManager.h"
#include "vkQueueSubmissionManager.h"
#include <cassert>

#include "vkException.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
QueueSubmission::QueueSubmission(core::ResourceManagerBase* resourceManager,
                                 core::ResourceId resourceId,
                                 core::ResourceSharedRef deviceRef,
                                 QueueSubmissionManager* manager,
                                 uint32_t queueFamilyIndex,
                                 CommandPoolFlagBits commandPoolFlags)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , batchNameToIndex_{}
  , manager_{ manager }
  , commandPool_{}
  , batches_{}
  , completionFrameIndex_{ 0 }
  , currentFrameIndex_{ 0 }
  , lastCompletedFrameIndex_{ 0 }
  , state_{}
  , vkSubmissions_{}
  , vkTimelineSubmissions_{}
  , timelineSemaphoreValues_{}
  , signalValues_{}
  , waitSemaphores_{}
  , vkCommandBuffers_{}
  , vkSignals_{}
  , dependencyCount_{ 0 }
  , commandBufferCount_{ 0 }
{
    state_.set(QueueSubmissionStateFlags::Initial);

    assert(deviceRef.valid());
    auto& device = deviceRef.as<gfx::Device>();

    commandPool_ = device.createCommandPool(queueFamilyIndex, commandPoolFlags);
}

void QueueSubmission::beginRecording()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    dependencyCount_ = 0;
    commandBufferCount_ = 0;

    assert(state_.value == metrix::value_cast(QueueSubmissionStateFlags::Initial));
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Recording);
}

void QueueSubmission::endRecording()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(state_.value == metrix::value_cast(QueueSubmissionStateFlags::Recording));
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Executable);
}

void QueueSubmission::beginBatchRecording(std::string_view batchName)
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(!state_.test(QueueSubmissionStateFlags::BatchRecording));
    state_.set(QueueSubmissionStateFlags::BatchRecording);

    auto index = batches_.size();
    auto& batch = batches_.emplace_back();

    batch.signal = manager_->acquireSignal(lastCompletedFrameIndex_);

    batchNameToIndex_.emplace(batchName, index);
}

void QueueSubmission::addBatchDependency(size_t fromBatch, PipelineStageFlagBits stageMask)
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(fromBatch < batches_.size());
    auto& srcBatch = batches_[fromBatch];
    auto& dstBatch = batches_.back();

    dstBatch.dependencies.emplace_back(srcBatch.signal, stageMask, currentFrameIndex_);

    dependencyCount_++;
}

void QueueSubmission::addBatchDependency(gfx::SubmissionBatchDependency const& externalDependency)
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    auto& dstBatch = batches_.back();
    dstBatch.dependencies.push_back(externalDependency);

    dependencyCount_++;
}

void QueueSubmission::addPresentationSignal(core::ResourceSharedRef const& signal)
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    auto& dstBatch = batches_.back();
    dstBatch.presentationSignal = signal;
}

void QueueSubmission::endBatchRecording()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(state_.test(QueueSubmissionStateFlags::BatchRecording));
    state_.reset(QueueSubmissionStateFlags::BatchRecording);
}

void QueueSubmission::beginCommandListRecording()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(!state_.test(QueueSubmissionStateFlags::CommandListRecording));
    state_.set(QueueSubmissionStateFlags::CommandListRecording);

    commandBufferCount_++;

    auto& batch = batches_.back();
    auto& pool = commandPool_.as<type_traits::platform_implementation_t<gfx::CommandPool>>();

    batch.commandLists.emplace_back(pool.allocCommandList());
}

auto QueueSubmission::commandListToRecord() -> gfx::CommandList&
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(state_.test(QueueSubmissionStateFlags::CommandListRecording));

    auto& batch = batches_.back();
    auto& commandList = batch.commandLists.back();

    return commandList;
}

void QueueSubmission::endCommandListRecording()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(state_.test(QueueSubmissionStateFlags::CommandListRecording));
    state_.reset(QueueSubmissionStateFlags::CommandListRecording);
}

auto QueueSubmission::signal() const -> core::ResourceSharedRef
{
    assert(!batches_.empty());
    return batches_.back().signal; // cause last batch finishes whole execution
}

auto QueueSubmission::waitOnCpu() -> uint64_t
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(state_.test(QueueSubmissionStateFlags::Pending));
    assert(signal().valid());

    auto completedValue = uint64_t{ 0 };

    if (signal().as<gfx::Signal>().waitOnCpu(completionFrameIndex_, std::numeric_limits<uint64_t>::max())) {
        state_.reset(QueueSubmissionStateFlags::Pending);

        if (!state_.test(QueueSubmissionStateFlags::Invalid)) {
            state_.value = metrix::value_cast(QueueSubmissionStateFlags::Executable);
        }

        completedValue = completionFrameIndex_;
    }

    return completedValue;
}

void QueueSubmission::reset()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    if (state_.test(QueueSubmissionStateFlags::Pending)) {
        throw std::runtime_error("attempt to reset queue commands in pending state");
    }

    if (state_.test(QueueSubmissionStateFlags::Recording)) {
        state_.set(QueueSubmissionStateFlags::Invalid);
        return;
    }

    for (auto& batch : batches_) {
        manager_->returnSignal(batch.signal);
    }
    batches_.clear();

    commandPool_.as<gfx::CommandPool>().reset();
    completionFrameIndex_ = 0;
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Initial);

    vkSubmissions_.clear();
    vkTimelineSubmissions_.clear();
    timelineSemaphoreValues_.clear();
    signalValues_.clear();
    waitSemaphores_.clear();
    waitStages_.clear();
    vkCommandBuffers_.clear();
    vkSignals_.clear();

    commandBufferCount_ = 0;
}

auto QueueSubmission::purpose() const -> multithreading::Purpose
{
    auto purpose = multithreading::Purpose{ multithreading::Purpose::General };

    auto& pool = commandPool_.as<type_traits::platform_implementation_t<gfx::CommandPool>>();
    auto& device = pool.device().as<type_traits::platform_implementation_t<gfx::Device>>();
    auto familyIndex = pool.queueFamilyIndex();

    if (familyIndex == device.graphicsQueueFamilyIndex()) {
        purpose = multithreading::Purpose::Render;
    } else if (familyIndex == device.computeQueueFamilyIndex()) {
        purpose = multithreading::Purpose::Compute;
    } else if (familyIndex == device.transferQueueFamilyIndex()) {
        purpose = multithreading::Purpose::Transfer;
    }
    assert(purpose != multithreading::Purpose::General);

    return purpose;
}

void QueueSubmission::submit()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    auto& pool = commandPool_.as<type_traits::platform_implementation_t<gfx::CommandPool>>();
    auto& device = pool.device().as<type_traits::platform_implementation_t<gfx::Device>>();

    auto queue = VkQueue{ VK_NULL_HANDLE };
    switch (submissionPurpose) {
        case multithreading::Purpose::Render:
            queue = device.graphicsQueue();
            break;
        case multithreading::Purpose::Transfer:
            queue = device.transferQueue();
            break;
        case multithreading::Purpose::Compute:
            queue = device.computeQueue();
            break;
        default:
            assert(false);
    }

    auto submissionBatchCount = batches_.size();

    vkSubmissions_.clear();
    vkSubmissions_.reserve(submissionBatchCount);

    vkTimelineSubmissions_.clear();
    vkTimelineSubmissions_.reserve(submissionBatchCount);

    timelineSemaphoreValues_.clear();
    timelineSemaphoreValues_.reserve(dependencyCount_);
    auto timelineValueOffset = size_t{ 0 };

    auto allWaitSemaphoreCount = dependencyCount_;
    auto waitSemaphoresOffset = size_t{ 0 };
    waitSemaphores_.clear();
    waitSemaphores_.reserve(allWaitSemaphoreCount);

    waitStages_.clear();
    waitStages_.reserve(allWaitSemaphoreCount);

    vkCommandBuffers_.clear();
    vkCommandBuffers_.reserve(commandBufferCount_);
    auto commandBuffersOffset = size_t{ 0 };

    vkSignals_.clear();
    vkSignals_.reserve(submissionBatchCount * 2); // max two signals per batch

    signalValues_.clear();
    signalValues_.reserve(submissionBatchCount * 2);

    auto signalsOffset = size_t{ 0 };

    for (auto const& batch : batches_) {
        assert(vkSubmissions_.size() < vkSubmissions_.capacity());
        assert(vkTimelineSubmissions_.size() < vkTimelineSubmissions_.capacity());

        auto& vkBatch = vkSubmissions_.emplace_back(VkSubmitInfo{});
        auto& vkTimelineSubmission = vkTimelineSubmissions_.emplace_back(VkTimelineSemaphoreSubmitInfo{});

        auto waitCount = batch.dependencies.size();
        for (auto const& dependency : batch.dependencies) {
            assert(timelineSemaphoreValues_.size() < timelineSemaphoreValues_.capacity());
            timelineSemaphoreValues_.push_back(dependency.completionValue());

            auto signalRef = dependency.signal().lock();
            assert(signalRef.valid());
            auto& vkWaitSignal = signalRef.as<type_traits::platform_implementation_t<gfx::Signal>>();

            assert(waitSemaphores_.size() < waitSemaphores_.capacity());
            waitSemaphores_.push_back(vkWaitSignal.handle());

            assert(waitStages_.size() < waitStages_.capacity());
            waitStages_.push_back(dependency.stageMask().cast_to<VkPipelineStageFlags>());
        }

        assert(batch.signal.valid());
        auto signalCount = size_t{ 1 };

        assert(signalValues_.size() < signalValues_.capacity());
        signalValues_.push_back(currentFrameIndex_);

        vkSignals_.push_back(batch.signal.as<type_traits::platform_implementation_t<gfx::Signal>>().handle());

        if (batch.presentationSignal.valid()) {
            signalCount++;
            vkSignals_.push_back(
              batch.presentationSignal.as<type_traits::platform_implementation_t<gfx::Signal>>().handle());

            assert(signalValues_.size() < signalValues_.capacity());
            signalValues_.push_back(1); // binary signal value (actually no sense)
        }

        vkTimelineSubmission.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
        vkTimelineSubmission.waitSemaphoreValueCount = waitCount;
        vkTimelineSubmission.pWaitSemaphoreValues = timelineSemaphoreValues_.data() + timelineValueOffset;
        vkTimelineSubmission.signalSemaphoreValueCount = signalCount;
        vkTimelineSubmission.pSignalSemaphoreValues = signalValues_.data() + signalsOffset; // signal values

        auto commandBufferCount = batch.commandLists.size();
        for (auto const& commandList : batch.commandLists) {
            auto const& vkCommandList = commandList.platformImplementation();

            assert(vkCommandBuffers_.size() < vkCommandBuffers_.capacity());
            vkCommandBuffers_.push_back(vkCommandList.handle());
        }

        vkBatch.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        vkBatch.pNext = &vkTimelineSubmission;
        vkBatch.waitSemaphoreCount = waitCount;
        vkBatch.pWaitSemaphores = waitSemaphores_.data() + waitSemaphoresOffset;
        vkBatch.pWaitDstStageMask = waitStages_.data() + waitSemaphoresOffset;
        vkBatch.commandBufferCount = commandBufferCount;
        vkBatch.pCommandBuffers = vkCommandBuffers_.data() + commandBuffersOffset;
        vkBatch.signalSemaphoreCount = signalCount;
        vkBatch.pSignalSemaphores = vkSignals_.data() + signalsOffset;

        timelineValueOffset += waitCount;
        waitSemaphoresOffset += waitCount;
        commandBuffersOffset += commandBufferCount;
        signalsOffset += signalCount;
    }

    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Pending);
    completionFrameIndex_ = currentFrameIndex_;

    if (auto vkResult = vkQueueSubmit(queue, vkSubmissions_.size(), vkSubmissions_.data(), VK_NULL_HANDLE);
        vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkQueueSubmit" };
    }
}

void QueueSubmission::setFrameIndices(uint64_t currentFrameIndex, uint64_t lastCompletedFrameIndex)
{
    assert(state_.test(QueueSubmissionStateFlags::Initial) || state_.test(QueueSubmissionStateFlags::Executable));
    currentFrameIndex_ = currentFrameIndex;
    lastCompletedFrameIndex_ = lastCompletedFrameIndex;
}
}
#endif
