//
// Created by anton on 10/8/25.
//

#include "queueSubmissionRecorder.h"
#include <cassert>

#include "multithreading/taskManager.h"
#include "multithreading/utility.h"

namespace cyclonite::gfx {
QueueSubmissionRecorder::QueueSubmissionRecorder()
  : submissionRef_{}
  , submission_{ nullptr }
  , futures_{}
{
}

QueueSubmissionRecorder::~QueueSubmissionRecorder()
{
    if (submissionRef_.valid()) {
        finish(true);
    }
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
        if (!submission_->isInRecordingState()) {
            throw std::runtime_error("submission must be in recording state to record new batch");
        }

        if (submission_->isInBatchRecordingState()) {
            throw std::runtime_error("batch recording must be over, before start new one");
        }

        submission_->beginBatchRecording();
    };

    futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));

    return batchRecorder;
}

void QueueSubmissionRecorder::finish(bool noexceptions)
{
    try {
        auto purpose = submission_->purpose();

        auto task = [this]() -> void {
            if (submission_->isInBatchRecordingState()) {
                throw std::runtime_error("all batch recording must be finished before.");
            }

            if (!submission_->isInRecordingState()) {
                throw std::runtime_error("submission recording is not started");
            }

            submission_->endRecording();
        };

        futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));

        multithreading::when_all(futures_).get();

        if (auto ex = multithreading::Executor::threadExecutor().taskManager().getLastException(); ex) {
            std::rethrow_exception(ex);
        }
    } catch (...) {
        if (noexceptions) {
            submission_->reset();
            return;
        } else {
            auto ex = std::current_exception();

            std::rethrow_exception(ex);
        }
    }

    submissionRef_ = core::ResourceSharedRef{};
}
}
