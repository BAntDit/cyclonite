
#include "submissionBatchRecorder.h"

namespace cyclonite::gfx {
SubmissionBatchRecorder::SubmissionBatchRecorder(QueueSubmissionRecorder* queueSubmissionRecorder)
  : queueSubmissionRecorder_{ queueSubmissionRecorder }
{
}
}
