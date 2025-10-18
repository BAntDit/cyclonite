
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
        auto purpose = queueSubmissionRecorder_->submission_->purpose();

        auto task = [recorder = queueSubmissionRecorder_, noexceptions]() -> void {
            if (!recorder->submission_->isInBatchRecordingState()) {
                throw std::runtime_error("batch recording is already finished");
            }

            recorder->submission_->endBatchRecording();
        };

        queueSubmissionRecorder_->futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));

        if (auto ex = multithreading::Executor::threadExecutor().taskManager().getLastException(); ex) {
            std::rethrow_exception(ex);
        }
    } catch (...) {
        if (noexceptions) {
            queueSubmissionRecorder_->submission_->reset();
            return;
        } else {
            auto ex = std::current_exception();

            std::rethrow_exception(ex);
        }
    }
}

void SubmissionBatchRecorder::addBatchDependency(size_t dependencyIndex, PipelineStageFlagBits stageMask)
{
    auto purpose = queueSubmissionRecorder_->submission_->purpose();

    auto task = [recorder = queueSubmissionRecorder_, dependencyIndex, stageMask]() -> void {
        if (!recorder->submission_->isInBatchRecordingState()) {
            throw std::runtime_error("batch recording is already finished");
        }

        recorder->submission_->addBatchDependency(dependencyIndex, stageMask);
    };

    queueSubmissionRecorder_->futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void SubmissionBatchRecorder::addBatchDependency(gfx::SubmissionBatchDependency const& externalDependency)
{
    auto purpose = queueSubmissionRecorder_->submission_->purpose();

    auto task = [recorder = queueSubmissionRecorder_, dep = externalDependency]() -> void {
        if (!recorder->submission_->isInBatchRecordingState()) {
            throw std::runtime_error("batch recording is already finished");
        }

        recorder->submission_->addBatchDependency(dep);
    };

    queueSubmissionRecorder_->futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

auto SubmissionBatchRecorder::submission() -> gfx::QueueSubmission&
{
    return *queueSubmissionRecorder_->submission_;
}

auto SubmissionBatchRecorder::submission() const -> gfx::QueueSubmission const&
{
    return *queueSubmissionRecorder_->submission_;
}

auto SubmissionBatchRecorder::futures() -> std::vector<std::future<void>>&
{
    return queueSubmissionRecorder_->futures_;
}

auto SubmissionBatchRecorder::SubmissionBatchRecorder::futures() const -> std::vector<std::future<void>> const&
{
    return queueSubmissionRecorder_->futures_;
}

auto SubmissionBatchRecorder::addCommandList() -> CommandListRecorder
{
    auto purpose = queueSubmissionRecorder_->submission_->purpose();

    auto commandListRecorder = CommandListRecorder{ this };

    auto task = [recorder = queueSubmissionRecorder_]() -> void {
        if (!recorder->submission_->isInRecordingState()) {
            throw std::runtime_error("submission must be in recording state to record new batch");
        }

        if (!recorder->submission_->isInBatchRecordingState()) {
            throw std::runtime_error("batch recording is not started yet");
        }

        if (recorder->submission_->isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording must be over before start new one");
        }

        recorder->submission_->beginCommandListRecording();
    };

    queueSubmissionRecorder_->futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));

    return commandListRecorder;
}
}
