//
// Created by anton on 10/4/25.
//

#ifndef CYCLONITE_GFX_QUEUE_SUBMISSION_INTERFACE_H
#define CYCLONITE_GFX_QUEUE_SUBMISSION_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "multithreading/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept QueueSubmissionConcept = requires(T t) {
    { t.beginRecording() } -> std::same_as<void>;

    { t.endRecording() } -> std::same_as<void>;

    { t.resourceBase() } -> std::same_as<core::ResourceBase*>;

    { t.isPending() } -> std::same_as<bool>;

    { t.isInInitialState() } -> std::same_as<bool>;

    { t.waitOnCpu() } -> std::same_as<uint64_t>;

    { t.purpose() } -> std::same_as<multithreading::Purpose>;
};

template<QueueSubmissionConcept PlatformImplementation>
class QueueSubmissionInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::beginRecording;
    using PlatformImplementation::endRecording;
    using PlatformImplementation::isPending;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::waitOnCpu;
    using PlatformImplementation::isInInitialState;
    using PlatformImplementation::purpose;
};
}

#endif // CYCLONITE_GFX_QUEUE_SUBMISSION_INTERFACE_H