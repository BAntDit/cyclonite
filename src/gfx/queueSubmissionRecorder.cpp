//
// Created by anton on 10/8/25.
//

#include "queueSubmissionRecorder.h"
#include <cassert>

#include "multithreading/taskManager.h"

namespace cyclonite::gfx {
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
        if (submission_->isInInitialState()) {
            throw std::runtime_error("submission must be in initial state for recording");
        }
        submission_->beginRecording();
    };

    if (multithreading::Executor::threadExecutor().matchesPurpose(purpose)) {
        task();
    } else {
        multithreading::TaskManager::submitTask(task, purpose).get();
    }
}
}
