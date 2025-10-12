//
// Created by anton on 10/12/25.
//

#ifndef CYCLONITE_COMMAND_LIST_RECORDER_H
#define CYCLONITE_COMMAND_LIST_RECORDER_H

namespace cyclonite::gfx {
class SubmissionBatchRecorder;

class CommandListRecorder
{
public:
    explicit CommandListRecorder(SubmissionBatchRecorder* batchRecorder);

private:
    SubmissionBatchRecorder* batchRecorder_;
};
}

#endif // CYCLONITE_COMMAND_LIST_RECORDER_H