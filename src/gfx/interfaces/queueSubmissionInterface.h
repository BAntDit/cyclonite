//
// Created by anton on 10/4/25.
//

#ifndef CYCLONITE_GFX_QUEUE_SUBMISSION_INTERFACE_H
#define CYCLONITE_GFX_QUEUE_SUBMISSION_INTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/commandList.h"
#include "gfx/common.h"
#include "multithreading/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept QueueSubmissionConcept = requires(T t, uint64_t idx, size_t batchIdx, PipelineStageFlagBits stageMask) {
    { t.beginRecording() } -> std::same_as<void>;

    { t.endRecording() } -> std::same_as<void>;

    { t.resourceBase() } -> std::same_as<core::ResourceBase*>;

    { t.isPending() } -> std::same_as<bool>;

    { t.isInInitialState() } -> std::same_as<bool>;

    { t.isInRecordingState() } -> std::same_as<bool>;

    { t.isInBatchRecordingState() } -> std::same_as<bool>;

    { t.isInCommandListRecordingState() } -> std::same_as<bool>;

    { t.waitOnCpu() } -> std::same_as<uint64_t>;

    { t.purpose() } -> std::same_as<multithreading::Purpose>;

    { t.beginBatchRecording(idx) } -> std::same_as<void>;

    { t.addBatchDependency(batchIdx, stageMask) } -> std::same_as<void>;

    { t.endBatchRecording() } -> std::same_as<void>;

    { t.beginCommandListRecording() } -> std::same_as<void>;

    { t.endCommandListRecording() } -> std::same_as<void>;

    { t.commandListToRecord() } -> std::same_as<gfx::CommandList>;

    { t.reset() } -> std::same_as<void>;

    { t.submit() } -> std::same_as<void>;
};

template<QueueSubmissionConcept PlatformImplementation>
class QueueSubmissionInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::addBatchDependency;
    using PlatformImplementation::beginBatchRecording;
    using PlatformImplementation::beginCommandListRecording;
    using PlatformImplementation::beginRecording;
    using PlatformImplementation::commandListToRecord;
    using PlatformImplementation::endBatchRecording;
    using PlatformImplementation::endCommandListRecording;
    using PlatformImplementation::endRecording;
    using PlatformImplementation::isInBatchRecordingState;
    using PlatformImplementation::isInCommandListRecordingState;
    using PlatformImplementation::isInInitialState;
    using PlatformImplementation::isInRecordingState;
    using PlatformImplementation::isPending;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::purpose;
    using PlatformImplementation::reset;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::submit;
    using PlatformImplementation::waitOnCpu;
};
}

#endif // CYCLONITE_GFX_QUEUE_SUBMISSION_INTERFACE_H