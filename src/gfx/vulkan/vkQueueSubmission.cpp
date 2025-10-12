
#include "vkQueueSubmission.h"
#include "gfx/commandPool.h"
#include "gfx/device.h"
#include "gfx/signal.h"
#include "multithreading/taskManager.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
QueueSubmission::QueueSubmission(core::ResourceManagerBase* resourceManager,
                                 core::ResourceId resourceId,
                                 core::ResourceSharedRef deviceRef,
                                 uint32_t queueFamilyIndex,
                                 CommandPoolFlagBits commandPoolFlags)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , commandPool_{}
  , batches_{}
  , competitionValue_{}
  , state_{}
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

void QueueSubmission::beginBatchRecording(uint64_t currentFrameIndex)
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(!state_.test(QueueSubmissionStateFlags::BatchRecording));
    state_.set(QueueSubmissionStateFlags::BatchRecording);

    auto& batch = batches_.emplace_back();
    auto& pool = commandPool_.as<type_traits::platform_implementation_t<gfx::CommandPool>>();
    auto& device = pool.device().as<gfx::Device>();

    batch.signal = device.createSignal(gfx::SignalType::TIMELINE, currentFrameIndex);
}

void QueueSubmission::addBatchDependency(size_t fromBatch, PipelineStageFlagBits stageMask)
{
    assert(fromBatch < batches_.size());
    auto& srcBatch = batches_[fromBatch];
    auto& dstBatch = batches_.back();

    dstBatch.dependencies.emplace_back(srcBatch.signal, stageMask);
}

void QueueSubmission::endBatchRecording()
{
    [[maybe_unused]] auto submissionPurpose = purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(submissionPurpose));

    assert(state_.test(QueueSubmissionStateFlags::BatchRecording));
    state_.reset(QueueSubmissionStateFlags::BatchRecording);
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

    if (signal().as<gfx::Signal>().waitOnCpu(competitionValue_, std::numeric_limits<uint64_t>::max())) {
        state_.reset(QueueSubmissionStateFlags::Pending);

        if (!state_.test(QueueSubmissionStateFlags::Invalid)) {
            state_.value = metrix::value_cast(QueueSubmissionStateFlags::Executable);
        }

        completedValue = competitionValue_;
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

    batches_.clear();
    commandPool_.as<gfx::CommandPool>().reset();
    competitionValue_ = 0;
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Initial);
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
}
#endif
