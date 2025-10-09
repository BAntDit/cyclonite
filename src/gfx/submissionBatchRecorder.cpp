
#include "submissionBatchRecorder.h"
#include "queueSubmissionRecorder.h"
#include <cassert>

#include "multithreading/taskManager.h"

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

void SubmissionBatchRecorder::finish(bool noexceptions/*= false*/)
{
    assert(queueSubmissionRecorder_);

    auto purpose = queueSubmissionRecorder_->submission_->purpose();

    auto task = [recorder = queueSubmissionRecorder_, noexceptions]() -> void { 
        if (!recorder->submission_->isInBatchRecordingState()) {
            recorder->submission_->reset();
            
            if (!noexceptions) {
                throw std::runtime_error("batch recording is already finished");
            } 
        }

        recorder->submission_->endBatchRecording();
    };
    
    queueSubmissionRecorder_->futures_.emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}
}
