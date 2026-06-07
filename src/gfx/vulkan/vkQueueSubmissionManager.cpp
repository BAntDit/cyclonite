//
// Created by anton on 10/7/25.
//

#include "vkQueueSubmissionManager.h"
#include "gfx/device.h"
#include "gfx/queueSubmission.h"
#include "multithreading/taskManager.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
QueueSubmissionManager::QueueSubmissionManager(Device* device)
  : device_{ device }
  , signalPool_{}
  , queueSubmissionRingMap_{}
  , completedFrames_{}
  , currentFrameIndex_{ 1 }
{
}

namespace {
auto getPurposeBits(multithreading::Purpose purpose) -> multithreading::PurposeBits
{
    assert(purpose != multithreading::Purpose::General);
    return multithreading::Executor::threadExecutor().taskManager().getExecutorPurposeBits(purpose);
}
}

auto QueueSubmissionManager::acquireSignal(uint64_t signalInitialValue) -> core::ResourceSharedRef
{
    auto result = core::ResourceSharedRef{};
    if (!signalPool_.empty()) {
        result = signalPool_.back();
        signalPool_.pop_back();
    } else {
        assert(device_ != nullptr);
        result = device_->createSignal(gfx::SignalType::TIMELINE, signalInitialValue);
    }

    return result;
}

void QueueSubmissionManager::returnSignal(core::ResourceSharedRef const& signal)
{
    signalPool_.push_back(signal);
}

auto QueueSubmissionManager::acquireQueueSubmission(multithreading::Purpose purpose,
                                                    CommandPoolFlagBits flags) -> core::ResourceSharedRef
{
    auto acquireQueueSubmissionTask = [purpose, flags, this]() -> core::ResourceSharedRef {
        assert(device_ != nullptr);

        assert(purpose != multithreading::Purpose::General);
        auto queueFamilyIndex = uint32_t{ 0 };

        switch (purpose) {
            case multithreading::Purpose::Render:
                queueFamilyIndex = device_->graphicsQueueFamilyIndex();
                break;
            case multithreading::Purpose::Compute:
                queueFamilyIndex = device_->computeQueueFamilyIndex();
                break;
            case multithreading::Purpose::Transfer:
                queueFamilyIndex = device_->transferQueueFamilyIndex();
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
                if (flags.test(gfx::CommandPoolFlags::TRANSIENT)) {
                    assert(submission.isExecutable());
                    submission.reset();
                }

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
            submissionRef = device_->createQueueSubmission(queueFamilyIndex, flags);
        }

        {
            auto* completedFrame = completedFrames_.at(purposeBits.value, queueFamilyIndex, flags.value);
            auto& submission = submissionRef.as<type_traits::platform_implementation_t<gfx::QueueSubmission>>();
            submission.setFrameIndices(currentFrameIndex_, completedFrame == nullptr ? 0 : *completedFrame);
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

auto QueueSubmissionManager::getCompletedFrameIndex(multithreading::Purpose purpose,
                                                    CommandPoolFlagBits flags) const -> uint64_t
{
    assert(device_ != nullptr);

    auto queueFamilyIndex = uint32_t{ 0 };

    switch (purpose) {
        case multithreading::Purpose::Render:
            queueFamilyIndex = device_->graphicsQueueFamilyIndex();
            break;
        case multithreading::Purpose::Compute:
            queueFamilyIndex = device_->computeQueueFamilyIndex();
            break;
        case multithreading::Purpose::Transfer:
            queueFamilyIndex = device_->transferQueueFamilyIndex();
            break;
        default:
            assert(false);
    }

    auto purposeBits = getPurposeBits(purpose);

    auto* completedFramePtr = completedFrames_.at(purposeBits.value, queueFamilyIndex, flags.value);

    return (completedFramePtr == nullptr) ? int64_t{ 0 } : *completedFramePtr;
}

void QueueSubmissionManager::flush()
{
    for (auto&& [_, submissions] : queueSubmissionRingMap_) {
        auto& submissionRef = submissions[currentFrameIndex_ % config_t::queue_submission_ring_size_v];

        if (!submissionRef.valid()) {
            continue;
        }

        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (submission.isExecutable()) {
            submission.submit();
        }
    }
    currentFrameIndex_++;
}
}
#endif
