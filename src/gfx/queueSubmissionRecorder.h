//
// Created by anton on 10/8/25.
//

#ifndef CYCLONITE_QUEUE_SUBMISSION_RECORDER_H
#define CYCLONITE_QUEUE_SUBMISSION_RECORDER_H

#include "core/resourceSharedRef.h"
#include "queueSubmission.h"
#include "submissionBatchRecorder.h"
#include <future>
#include <vector>

namespace cyclonite::gfx {
class QueueSubmissionRecorder
{
    friend class SubmissionBatchRecorder;

public:
    QueueSubmissionRecorder();

    ~QueueSubmissionRecorder();

    void setQueueSubmission(core::ResourceSharedRef submissionRef);

    void setFrameNumber(uint64_t currentFrameNumber) { currentFrameIndex_ = currentFrameNumber; }

    [[nodiscard]] auto addBatch() -> SubmissionBatchRecorder;

    void finish() { finish(false); }

private:
    void finish(bool noexceptions);

    core::ResourceSharedRef submissionRef_;
    QueueSubmission* submission_;
    std::vector<std::future<void>> futures_;
    uint64_t currentFrameIndex_;
};
}

#endif // CYCLONITE_QUEUE_SUBMISSION_RECORDER_H