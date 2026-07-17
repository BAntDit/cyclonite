//
// Created by anton on 10/8/25.
//

#ifndef CYCLONITE_QUEUE_SUBMISSION_RECORDER_H
#define CYCLONITE_QUEUE_SUBMISSION_RECORDER_H

#include "core/resourceSharedRef.h"
#include "queueSubmission.h"
#include "submissionBatchRecorder.h"

namespace cyclonite::gfx {
class QueueSubmissionRecorder
{
    friend class SubmissionBatchRecorder;
    friend class CommandListRecorder;

public:
    explicit QueueSubmissionRecorder(core::ResourceSharedRef const& submissionRef);

    ~QueueSubmissionRecorder() = default;

    [[nodiscard]] auto addBatch(std::string_view batchName) -> SubmissionBatchRecorder;

    void start();

    [[nodiscard]] auto finish() -> std::future<void>;

    [[nodiscard]] auto flush() -> std::future<void>;

private:
    void addTask(std::future<void>&& recordingTask) { taskFutures_.emplace_back(std::move(recordingTask)); }

    core::ResourceSharedRef submissionRef_;
    std::vector<std::future<void>> taskFutures_;
};
}

#endif // CYCLONITE_QUEUE_SUBMISSION_RECORDER_H