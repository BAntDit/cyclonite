
#include "submissionBatchRecorder.h"
#include "multithreading/taskManager.h"
#include "queueSubmissionRecorder.h"

namespace cyclonite::gfx {
SubmissionBatchRecorder::SubmissionBatchRecorder(QueueSubmissionRecorder* queueSubmissionRecorder)
  : queueSubmissionRecorder_{ queueSubmissionRecorder }
{
}

SubmissionBatchRecorder::~SubmissionBatchRecorder()
{
    if (queueSubmissionRecorder_ != nullptr) {
        finish(true);
        queueSubmissionRecorder_ = nullptr;
    }
}

void SubmissionBatchRecorder::finish(bool noexceptions)
{
    try {
        auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
        auto purpose = submission.purpose();

        auto task = [submissionRef = queueSubmissionRecorder_->submissionRef_]() mutable -> void {
            auto& submission = submissionRef.as<gfx::QueueSubmission>();
            if (!submission.isInBatchRecordingState()) {
                throw std::runtime_error("batch recording is already finished");
            }
            submission.endBatchRecording();
        };

        queueSubmissionRecorder_->addTask(multithreading::TaskManager::submitTask(task, purpose));

        if (auto ex = multithreading::Executor::threadExecutor().taskManager().getLastException(); ex) {
            std::rethrow_exception(ex);
        }
    } catch (...) {
        if (noexceptions) {
            auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
            submission.reset();
            return;
        } else {
            auto ex = std::current_exception();

            std::rethrow_exception(ex);
        }
    }
    queueSubmissionRecorder_ = nullptr;
}

void SubmissionBatchRecorder::addBatchDependency(size_t dependencyIndex, PipelineStageFlagBits stageMask)
{
    auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission.purpose();

    auto task =
      [submissionRef = queueSubmissionRecorder_->submissionRef_, dependencyIndex, stageMask]() mutable -> void {
        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (!submission.isInBatchRecordingState()) {
            throw std::runtime_error("batch is not in recording state");
        }

        submission.addBatchDependency(dependencyIndex, stageMask);
    };

    queueSubmissionRecorder_->addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void SubmissionBatchRecorder::addPresentationSignal(core::ResourceSharedRef const& signal)
{
    auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
    [[maybe_unused]] auto purpose = submission.purpose();
    assert(multithreading::Executor::threadExecutor().matchesPurpose(purpose));

    submission.addPresentationSignal(signal);
}

void SubmissionBatchRecorder::addBatchDependency(gfx::SubmissionBatchDependency const& externalDependency)
{
    auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission.purpose();

    auto task = [submissionRef = queueSubmissionRecorder_->submissionRef_, dep = externalDependency]() mutable -> void {
        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (!submission.isInBatchRecordingState()) {
            throw std::runtime_error("batch is not in recording state");
        }
        submission.addBatchDependency(dep);
    };

    queueSubmissionRecorder_->addTask(multithreading::TaskManager::submitTask(task, purpose));
}

auto SubmissionBatchRecorder::submission() -> gfx::QueueSubmission&
{
    auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
    return submission;
}

auto SubmissionBatchRecorder::submission() const -> gfx::QueueSubmission const&
{
    auto const& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
    return submission;
}

auto SubmissionBatchRecorder::addCommandList() -> CommandListRecorder
{
    auto& submission = queueSubmissionRecorder_->submissionRef_.as<gfx::QueueSubmission>();
    auto purpose = submission.purpose();

    auto commandListRecorder = CommandListRecorder{ this };

    auto task = [submissionRef = queueSubmissionRecorder_->submissionRef_]() mutable -> void {
        auto& submission = submissionRef.as<gfx::QueueSubmission>();
        if (!submission.isInRecordingState()) {
            throw std::runtime_error("submission must be in recording state to record new batch");
        }

        if (!submission.isInBatchRecordingState()) {
            throw std::runtime_error("batch recording is not started yet");
        }

        if (submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording must be over before start new one");
        }

        submission.beginCommandListRecording();
    };

    queueSubmissionRecorder_->addTask(multithreading::TaskManager::submitTask(task, purpose));

    return commandListRecorder;
}
}
