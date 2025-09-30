
#ifndef CYCLONITE_SUBMISSION_BATCH_DEPENDECY_INTERFACE_H
#define CYCLONITE_SUBMISSION_BATCH_DEPENDECY_INTERFACE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces 
{
template<typename T>
concept SubmissionBatchDependencyConcept = requires(T t)
{ 
	{
        t.signal()
    }
    ->std::same_as<core::ResourceWeakRef>;

    {
        t.stageMask()
    }
    ->std::same_as<PipelineStageFlagBits>;
};

template<SubmissionBatchDependencyConcept PlatformImplementation>
class SubmissionBatchDependencyInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::signal;
    using PlatformImplementation::stageMask;
};
}

#endif // CYCLONITE_SUBMISSION_BATCH_DEPENDECY_INTERFACE_H