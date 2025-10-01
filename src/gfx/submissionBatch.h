//
// Created by anton on 10/1/25.
//

#ifndef CYCLONITE_GFX_SUBMISSIONBATCH_H
#define CYCLONITE_GFX_SUBMISSIONBATCH_H

#include "submissionBatchDependency.h"
#include "commandList.h"
#include "core/resourceSharedRef.h"
#include <vector>

namespace cyclonite::gfx
{
struct SubmissionBatch
{
    std::vector<gfx::SubmissionBatchDependency> dependencies;
    std::vector<gfx::CommandList> commandLists;
    core::ResourceSharedRef signal;
};
}

#endif // CYCLONITE_GFX_SUBMISSIONBATCH_H