//
// Created by anton on 10/18/25.
//

#ifndef CYCLONITE_QUEUE_SUBMISSION_MANAGER_INTERFACE_H
#define CYCLONITE_QUEUE_SUBMISSION_MANAGER_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "multithreading/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept QueueSubmissionManagerConcept = requires(T t, multithreading::Purpose p, CommandPoolFlagBits f) {
    { t.currentFrameIndex() } -> std::same_as<uint64_t>;

    { t.acquireQueueSubmission(p, f) } -> std::same_as<core::ResourceSharedRef>;

    { t.flush() } -> std::same_as<void>;
};

template<QueueSubmissionManagerConcept PlatformImplementation>
class QueueSubmissionManagerInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::acquireQueueSubmission;
    using PlatformImplementation::currentFrameIndex;
    using PlatformImplementation::flush;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::platformQueueSubmissionManager;
};
};

#endif // CYCLONITE_QUEUE_SUBMISSION_MANAGER_INTERFACE_H
