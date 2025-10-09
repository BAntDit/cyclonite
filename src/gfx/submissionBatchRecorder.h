
#ifndef CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H
#define CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H

namespace cyclonite::gfx 
{
class QueueSubmissionRecorder;

class SubmissionBatchRecorder
{
    friend class QueueSubmissionRecorder;

private:
    SubmissionBatchRecorder(QueueSubmissionRecorder* queueSubmissionRecorder);

    ~SubmissionBatchRecorder();

    void finish(bool noexceptions = false);

    // TODO:: add command list 

private:
    QueueSubmissionRecorder* queueSubmissionRecorder_;
};
}

#endif // CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H
