//
// Created by anton on 10/7/25.
//

#ifndef CYCLONITE_VK_QUEUE_SUBMISSION_MANAGER_H
#define CYCLONITE_VK_QUEUE_SUBMISSION_MANAGER_H

#include "core/hashTable.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/config.h"
#include "multithreading/common.h"
#include <array>

#include "vkQueueSubmission.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Device;

class QueueSubmissionManager
{
    friend class vulkan::Device;

public:
    QueueSubmissionManager(QueueSubmissionManager const&) = delete;

    QueueSubmissionManager(QueueSubmissionManager&&) = default;

    ~QueueSubmissionManager() = default;

    auto operator=(QueueSubmissionManager const&) -> QueueSubmissionManager& = delete;

    auto operator=(QueueSubmissionManager&&) -> QueueSubmissionManager& = default;

    [[nodiscard]] auto currentFrameNumber() const -> uint64_t { return currentFrameNumber_; }

    [[nodiscard]] auto currentSubmissionIndex() const -> uint64_t
    {
        return currentFrameNumber_ % config_t::queue_submission_ring_size_v;
    }

    [[nodiscard]] auto acquireQueueSubmission(multithreading::Purpose purpose,
                                              CommandPoolFlagBits flags,
                                              uint16_t priorityGroup) -> core::ResourceSharedRef;

    [[nodiscard]] auto completedFrameNumber(multithreading::Purpose purpose,
                                            CommandPoolFlagBits flags,
                                            uint16_t priorityGroup) const -> uint64_t;

    void flush();

    void reset();

    [[nodiscard]] auto acquireSignal(uint64_t signalInitialValue) -> core::ResourceSharedRef;

    void returnSignal(core::ResourceSharedRef const& signal);

    [[nodiscard]] auto platformQueueSubmissionManager() -> vulkan::QueueSubmissionManager* { return this; };

    [[nodiscard]] auto platformQueueSubmissionManager() const -> vulkan::QueueSubmissionManager const* { return this; };

private:
    explicit QueueSubmissionManager(Device* device);

    using queue_submission_ring_t = std::array<core::ResourceSharedRef, config_t::queue_submission_ring_size_v>;

    using queue_submission_map_t = core::StaticHashTable<queue_submission_ring_t,
                                                         config_t::max_queue_submission_ring_count_v,
                                                         std::underlying_type_t<multithreading::Purpose>,
                                                         uint32_t, // queue family index
                                                         uint16_t, // priority group
                                                         std::underlying_type_t<gfx::CommandPoolFlags>>;

    using completion_map_t = core::StaticHashTable<uint64_t,
                                                   config_t::max_queue_submission_ring_count_v,
                                                   std::underlying_type_t<multithreading::Purpose>,
                                                   uint32_t, // queue family index
                                                   uint16_t, // priority group
                                                   std::underlying_type_t<gfx::CommandPoolFlags>>;

    Device* device_;

    std::vector<core::ResourceSharedRef> signalPool_;

    queue_submission_map_t queueSubmissionRingMap_;
    completion_map_t completedFrames_;

    uint64_t currentFrameNumber_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_QUEUE_SUBMISSION_MANAGER_H
