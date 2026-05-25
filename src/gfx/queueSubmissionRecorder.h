//
// Created by anton on 10/8/25.
//

#ifndef CYCLONITE_QUEUE_SUBMISSION_RECORDER_H
#define CYCLONITE_QUEUE_SUBMISSION_RECORDER_H

#include "core/resourceSharedRef.h"
#include "frameRecordingContext.h"
#include "queueSubmission.h"
#include "submissionBatchRecorder.h"

namespace cyclonite::gfx {
class QueueSubmissionRecorder
{
    friend class SubmissionBatchRecorder;

public:
    QueueSubmissionRecorder(core::ResourceSharedRef const& submissionRef, FrameRecordingContext& frameRecordingContext);

    ~QueueSubmissionRecorder() = default;

    [[nodiscard]] auto addBatch() -> SubmissionBatchRecorder;

    void start();

    void finish(bool releaseSubmissionOnly = false);

private:
    core::ResourceSharedRef submissionRef_;
    FrameRecordingContext* recordingContext_;
};
}

#endif // CYCLONITE_QUEUE_SUBMISSION_RECORDER_H