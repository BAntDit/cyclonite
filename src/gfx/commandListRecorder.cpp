//
// Created by anton on 10/12/25.
//

#include "commandListRecorder.h"

#include "multithreading/taskManager.h"
#include "queueSubmissionRecorder.h"
#include "submissionBatchRecorder.h"

namespace cyclonite::gfx {
CommandListRecorder::CommandListRecorder(SubmissionBatchRecorder* batchRecorder)
  : batchRecorder_{ batchRecorder }
{
}

CommandListRecorder::~CommandListRecorder()
{
    if (batchRecorder_ != nullptr) {
        finish(true);
        batchRecorder_ = nullptr;
    }
}

void CommandListRecorder::begin(CommandListUsageFlagBits usage)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, usage]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().begin(usage);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::beginRenderPass(core::ResourceSharedRef renderPassRef)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, renderPassRef]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().beginRenderPass(renderPassRef);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::endRenderPass()
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().endRenderPass();
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::end()
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().end();
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::finish(bool noexceptions /* = false*/)
{
    try {
        auto purpose = batchRecorder_->submission().purpose();

        auto task = [recorder = batchRecorder_, noexceptions]() -> void {
            if (!recorder->submission().isInCommandListRecordingState()) {
                throw std::runtime_error("command list recording is already finished");
            }

            recorder->submission().endCommandListRecording();
        };

        batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));

        if (auto ex = multithreading::Executor::threadExecutor().taskManager().getLastException(); ex) {
            std::rethrow_exception(ex);
        }
    } catch (...) {
        if (noexceptions) {
            batchRecorder_->submission().reset();
            return;
        } else {
            auto ex = std::current_exception();

            std::rethrow_exception(ex);
        }
    }
}
}
