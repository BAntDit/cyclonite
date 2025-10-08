//
// Created by anton on 10/7/25.
//

#include "vkQueueSubmissionManager.h"
#include "gfx/device.h"
#include "gfx/queueSubmission.h"
#include "multithreading/taskManager.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
QueueSubmissionManager::QueueSubmissionManager(core::ResourceSharedRef deviceRef)
  : deviceRef_{ deviceRef }
  , queueSubmissionRingMap_{}
  , currentFrameIndex_{ 0 }
{
}

namespace {
auto getPurposeBits(multithreading::Purpose purpose) -> multithreading::PurposeBits
{
    assert(purpose != multithreading::Purpose::General);
    return multithreading::Executor::threadExecutor().taskManager().getExecutorPurposeBits(purpose);
}
}

auto QueueSubmissionManager::acquireQueueSubmission(multithreading::Purpose purpose,
                                                    CommandPoolFlagBits flags) -> core::ResourceSharedRef
{
    auto acquireQueueSubmissionTask = [purpose, flags, this]() -> core::ResourceSharedRef {
        assert(deviceRef_.valid());

        assert(purpose != multithreading::Purpose::General);
        auto queueFamilyIndex = uint32_t{ 0 };

        auto& device = deviceRef_.as<type_traits::platform_implementation_t<gfx::Device>>();

        switch (purpose) {
            case multithreading::Purpose::Render:
                queueFamilyIndex = device.graphicsQueueFamilyIndex();
                break;
            case multithreading::Purpose::Compute:
                queueFamilyIndex = device.computeQueueFamilyIndex();
                break;
            case multithreading::Purpose::Transfer:
                queueFamilyIndex = device.transferQueueFamilyIndex();
                break;
            default:
                assert(false);
        }

        auto purposeBits = getPurposeBits(purpose);

        auto it = queueSubmissionRingMap_.find(purposeBits.value, queueFamilyIndex, flags.value);
        if (it == queueSubmissionRingMap_.end()) {
            auto [newIt, success] =
              queueSubmissionRingMap_.add(queue_submission_ring_t{}, purposeBits.value, queueFamilyIndex, flags.value);

            if (!success) {
                throw std::runtime_error("failed to emplace new queue submission ring");
            }

            it = newIt;
        }

        auto&& [_, submissions] = *it;
        auto& submissionRef = submissions[currentFrameIndex_ % config_t::queue_submission_ring_size_v];

        if (submissionRef.valid()) {
            auto& submission = submissionRef.as<gfx::QueueSubmission>();
            if (submission.isPending()) {
                auto submissionFrame = submission.waitOnCpu();

                if (auto completionIt = completedFrames_.find(purposeBits.value, queueFamilyIndex, flags.value);
                    completionIt != completedFrames_.end()) {
                    auto& [k, completedFrame] = *completionIt;
                    completedFrame = submissionFrame;
                } else {
                    [[maybe_unused]] auto [newIt, success] =
                      completedFrames_.add(submissionFrame, purposeBits.value, queueFamilyIndex, flags.value);
                    assert(success);
                }
            } // if pending
        } else { // new submission
            submissionRef = device.createQueueSubmission(queueFamilyIndex, flags);
        }

        return submissionRef;
    };

    auto queueSubmissionRef = core::ResourceSharedRef{};
    if (multithreading::Executor::threadExecutor().matchesPurpose(purpose)) {
        queueSubmissionRef = acquireQueueSubmissionTask();
    } else {
        queueSubmissionRef = multithreading::TaskManager::submitTask(acquireQueueSubmissionTask, purpose).get();
    }

    return queueSubmissionRef;
}
}
#endif
