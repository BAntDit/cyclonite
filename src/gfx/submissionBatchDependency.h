//
// Created by anton on 9/28/25.
//

#ifndef CYCLONITE_GFX_SUBMISSION_BATCH_DEPENDENCY_H
#define CYCLONITE_GFX_SUBMISSION_BATCH_DEPENDENCY_H

#include "common.h"
#include "core/resourceWeakRef.h"

namespace cyclonite::gfx {
struct SubmissionBatchDependency
{
    SubmissionBatchDependency(core::ResourceWeakRef dependencySignal, PipelineStageFlagBits dependencyStageMask)
      : signal{ dependencySignal }
      , stageMask{ dependencyStageMask }
    {
    }

    core::ResourceWeakRef signal;
    PipelineStageFlagBits stageMask;
};
}

#endif // CYCLONITE_GFX_SUBMISSION_BATCH_DEPENDENCY_H