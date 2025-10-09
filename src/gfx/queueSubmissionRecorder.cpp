//
// Created by anton on 10/8/25.
//

#include "queueSubmissionRecorder.h"
#include <cassert>

#include "multithreading/taskManager.h"

namespace cyclonite::gfx {
QueueSubmissionRecorder::QueueSubmissionRecorder()
  : submissionRef_{}
  , submission_{ nullptr }
  , futures_{}
  , currentFrameIndex_{ std::numeric_limits<uint64_t>::max() }
{
}

void QueueSubmissionRecorder::setQueueSubmission(core::ResourceSharedRef submissionRef)
{
    if (submissionRef_.valid()) {
        throw std::runtime_error("previous submission recording is not finished");
    }

    submissionRef_ = submissionRef;
    assert(submissionRef.valid());

    submission_ = &submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission_->purpose();

    auto task = [=, this]() -> void {
        if (!submission_->isInInitialState()) {
            throw std::runtime_error("submission must be in initial state for recording");
        }
        submission_->beginRecording();
    };

    futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

auto QueueSubmissionRecorder::addBatch() -> SubmissionBatchRecorder
{
    assert(submission_ != nullptr);
    auto batchRecorder = SubmissionBatchRecorder{ this };

    auto purpose = submission_->purpose();

    auto task = [this]() -> void {
        if (currentFrameIndex_ == std::numeric_limits<uint64_t>::max()) {
            throw std::runtime_error("invalid frame index");
        }

        if (!submission_->isInRecordingState()) {
            throw std::runtime_error("submission must be in recording state to record new batch");
        }

        if (submission_->isInBatchRecordingState()) {
            throw std::runtime_error("batch recording must be over, before start new one");
        }

        submission_->beginBatchRecording(currentFrameIndex_);
    };

    futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));

    return batchRecorder;
}
}
