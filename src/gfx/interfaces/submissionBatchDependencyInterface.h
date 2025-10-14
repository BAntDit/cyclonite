
#ifndef CYCLONITE_SUBMISSION_BATCH_DEPENDECY_INTERFACE_H
#define CYCLONITE_SUBMISSION_BATCH_DEPENDECY_INTERFACE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept SubmissionBatchDependencyConcept = requires(T t) {
    { t.signal() } -> std::same_as<core::ResourceWeakRef>;

    { t.stageMask() } -> std::same_as<PipelineStageFlagBits>;

    { t.value() } -> std::same_as<uint64_t>;

    { t.completionValue() } -> std::same_as<uint64_t>;

    { t.type() } -> std::same_as<SignalType>;
};

template<SubmissionBatchDependencyConcept PlatformImplementation>
class SubmissionBatchDependencyInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::signal;
    using PlatformImplementation::stageMask;
    using PlatformImplementation::type;
    using PlatformImplementation::value;
};
}

#endif // CYCLONITE_SUBMISSION_BATCH_DEPENDECY_INTERFACE_H