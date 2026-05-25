
#ifndef CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H
#define CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H

#include "commandListRecorder.h"
#include "frameRecordingContext.h"
#include "gfx/common.h"
#include "gfx/queueSubmission.h"

namespace cyclonite::gfx {
class QueueSubmissionRecorder;

class SubmissionBatchRecorder
{
    friend class QueueSubmissionRecorder;
    friend class CommandListRecorder;

public:
    ~SubmissionBatchRecorder();

    void finish() { finish(false); }

    void addBatchDependency(size_t dependencyIndex, PipelineStageFlagBits stageMask);

    void addBatchDependency(gfx::SubmissionBatchDependency const& externalDependency);

    [[nodiscard]] auto addCommandList() -> CommandListRecorder;

    [[nodiscard]] auto submission() -> gfx::QueueSubmission&;

    [[nodiscard]] auto submission() const -> gfx::QueueSubmission const&;

private:
    SubmissionBatchRecorder(QueueSubmissionRecorder* queueSubmissionRecorder,
                            FrameRecordingContext* frameRecordingContext);

    void addPresentationSignal(core::ResourceSharedRef const& signal);

private:
    void finish(bool noexceptions);

    QueueSubmissionRecorder* queueSubmissionRecorder_;
    FrameRecordingContext* frameRecordingContext_;
};
}

#endif // CYCLONITE_GFX_SUBMISSION_BATCH_RECORDER_H
