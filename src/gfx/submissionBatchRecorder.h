
#ifndef CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H
#define CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H

#include "commandListRecorder.h"
#include "gfx/common.h"

namespace cyclonite::gfx {
class QueueSubmissionRecorder;

class SubmissionBatchRecorder
{
    friend class QueueSubmissionRecorder;

private:
    explicit SubmissionBatchRecorder(QueueSubmissionRecorder* queueSubmissionRecorder);

    ~SubmissionBatchRecorder();

    void finish() { finish(false); }

    void addBatchDependency(size_t dependencyIndex, PipelineStageFlagBits stageMask);

    [[nodiscard]] auto addCommandList() -> CommandListRecorder;

private:
    void finish(bool noexceptions);

    QueueSubmissionRecorder* queueSubmissionRecorder_;
};
}

#endif // CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H
