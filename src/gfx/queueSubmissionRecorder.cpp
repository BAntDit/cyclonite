//
// Created by anton on 10/8/25.
//

#include "queueSubmissionRecorder.h"
#include <cassert>

#include "multithreading/taskManager.h"
#include "multithreading/utility.h"

namespace cyclonite::gfx {
QueueSubmissionRecorder::QueueSubmissionRecorder(core::ResourceSharedRef const& submissionRef,
                                                 FrameRecordingContext& frameRecordingContext)
  : submissionRef_{ submissionRef }
  , recordingContext_{ &frameRecordingContext }
{
}

void QueueSubmissionRecorder::start()
{
    assert(submissionRef_.valid());

    auto& submission = submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission.purpose();

    auto task = [submissionRef = submissionRef_]() mutable -> void {
        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (submission.isInInitialState()) {
            submission.beginRecording();
        }

        if (!submission.isInRecordingState() || submission.isInBatchRecordingState()) {
            throw std::runtime_error("submission is in wrong state");
        }
    };

    recordingContext_->addTask(multithreading::TaskManager::submitTask(task, purpose));
}

auto QueueSubmissionRecorder::addBatch() -> SubmissionBatchRecorder
{
    auto batchRecorder = SubmissionBatchRecorder{ this, recordingContext_ };

    auto& submission = submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission.purpose();

    auto task = [submissionRef = submissionRef_]() mutable -> void {
        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (!submission.isInRecordingState()) {
            throw std::runtime_error("submission must be in recording state to record new batch");
        }
        if (submission.isInBatchRecordingState()) {
            throw std::runtime_error("batch recording must be over, before start new one");
        }
        submission.beginBatchRecording();
    };
    recordingContext_->addTask(multithreading::TaskManager::submitTask(task, purpose));

    return batchRecorder;
}

void QueueSubmissionRecorder::finish(bool releaseSubmissionOnly /* = false*/)
{
    if (!releaseSubmissionOnly) {
        auto& submission = submissionRef_.as<gfx::QueueSubmission>();
        auto purpose = submission.purpose();

        auto task = [submissionRef = submissionRef_]() mutable -> void {
            auto& submission = submissionRef.as<gfx::QueueSubmission>();
            if (submission.isInBatchRecordingState()) {
                throw std::runtime_error("all batch recording must be finished before.");
            }
            if (!submission.isInRecordingState()) {
                throw std::runtime_error("submission recording is not started");
            }
            submission.endRecording();
        };

        recordingContext_->addTask(multithreading::TaskManager::submitTask(task, purpose));

        auto tasks = std::move(recordingContext_->recordingTasks());
        multithreading::when_all(tasks).get();

        if (auto ex = multithreading::Executor::threadExecutor().taskManager().getLastException(); ex) {
            std::rethrow_exception(ex);
        }
    }

    submissionRef_ = core::ResourceSharedRef{};
    recordingContext_ = nullptr;
}
}
