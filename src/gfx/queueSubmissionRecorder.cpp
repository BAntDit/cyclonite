//
// Created by anton on 10/8/25.
//

#include "queueSubmissionRecorder.h"
#include <cassert>

#include "multithreading/taskManager.h"
#include "multithreading/utility.h"

namespace cyclonite::gfx {
QueueSubmissionRecorder::QueueSubmissionRecorder(core::ResourceSharedRef const& submissionRef)
  : submissionRef_{ submissionRef }
  , taskFutures_{}
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

    addTask(multithreading::TaskManager::submitTask(task, purpose));
}

auto QueueSubmissionRecorder::addBatch(std::string_view batchName) -> SubmissionBatchRecorder
{
    auto batchRecorder = SubmissionBatchRecorder{ this };

    auto& submission = submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission.purpose();

    auto task = [submissionRef = submissionRef_, batchName = std::string{ batchName }]() mutable -> void {
        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (!submission.isInRecordingState()) {
            throw std::runtime_error("submission must be in recording state to record new batch");
        }
        if (submission.isInBatchRecordingState()) {
            throw std::runtime_error("batch recording must be over, before start new one");
        }
        submission.beginBatchRecording(batchName);
    };
    addTask(multithreading::TaskManager::submitTask(task, purpose));

    return batchRecorder;
}

auto QueueSubmissionRecorder::finish() -> std::future<void>
{

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

    addTask(multithreading::TaskManager::submitTask(task, purpose));

    return flush();
}

auto QueueSubmissionRecorder::flush() -> std::future<void>
{
    auto tasks = std::vector{ std::move(taskFutures_) };
    taskFutures_.clear();

    return multithreading::when_all(tasks);
}
}
