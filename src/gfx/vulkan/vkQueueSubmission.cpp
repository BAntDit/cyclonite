
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
    // assert(multithreading::Executor::threadExecutor().p);

    state_.set(QueueSubmissionStateFlags::Initial);

    assert(deviceRef.valid());
    auto& device = deviceRef.as<gfx::Device>();

    commandPool_ = device.createCommandPool(queueFamilyIndex, commandPoolFlags);
}

void QueueSubmission::beginRecording()
{
    // assert(multithreading::Executor::isInRenderThread());
    assert(state_.value == metrix::value_cast(QueueSubmissionStateFlags::Initial));
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Recording);
}

void QueueSubmission::endRecording()
{
    // assert(multithreading::Executor::isInRenderThread());
    assert(state_.value == metrix::value_cast(QueueSubmissionStateFlags::Recording));
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Executable);
}

auto QueueSubmission::signal() const -> core::ResourceSharedRef
{
    assert(!batches_.empty());
    return batches_.back().signal; // cause last batch finishes whole execution
}

void QueueSubmission::waitOnCpu()
{
    // assert(multithreading::Executor::isInRenderThread());
    assert(state_.test(QueueSubmissionStateFlags::Pending));
    assert(signal().valid());

    if (signal().as<gfx::Signal>().waitOnCpu(competitionValue_, std::numeric_limits<uint64_t>::max())) {
        state_.reset(QueueSubmissionStateFlags::Pending);

        if (!state_.test(QueueSubmissionStateFlags::Invalid)) {
            state_.value = metrix::value_cast(QueueSubmissionStateFlags::Executable);
        }
    }
}

void QueueSubmission::reset()
{
    assert(multithreading::Executor::isInRenderThread());

    if (state_.test(QueueSubmissionStateFlags::Pending)) {
        state_.set(QueueSubmissionStateFlags::Invalid);
        return;
    }

    batches_.clear();
    commandPool_.as<gfx::CommandPool>().reset();
    competitionValue_ = 0;
    state_.value = metrix::value_cast(QueueSubmissionStateFlags::Initial);
}
}
#endif
