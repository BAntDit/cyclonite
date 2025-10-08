//
// Created by anton on 10/8/25.
//

#ifndef CYCLONITE_QUEUE_SUBMISSION_RECORDER_H
#define CYCLONITE_QUEUE_SUBMISSION_RECORDER_H

#include "core/resourceSharedRef.h"
#include "gfx/queueSubmission.h"

namespace cyclonite::gfx
{
class QueueSubmissionRecorder
{
public:
    QueueSubmissionRecorder() = default;

    ~QueueSubmissionRecorder() { finish(); }

    void setQueueSubmission(core::ResourceSharedRef submissionRef);

    // TODO:: add batch -> batch recorder

    void finish();

private:
    core::ResourceSharedRef submissionRef_;
    QueueSubmission* submission_;
};
}

#endif //CYCLONITE_QUEUE_SUBMISSION_RECORDER_H