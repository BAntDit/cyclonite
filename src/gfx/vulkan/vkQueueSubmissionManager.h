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

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class QueueSubmissionManager
{
public:
    QueueSubmissionManager(core::ResourceSharedRef deviceRef);

    QueueSubmissionManager(QueueSubmissionManager const&) = delete;

    QueueSubmissionManager(QueueSubmissionManager&&) = default;

    ~QueueSubmissionManager() = default;

    auto operator=(QueueSubmissionManager const&) -> QueueSubmissionManager& = delete;

    auto operator=(QueueSubmissionManager&&) -> QueueSubmissionManager& = default;

    [[nodiscard]] auto currentFrameIndex() const -> uint64_t { return currentFrameIndex_; }

    [[nodiscard]] auto acquireQueueSubmission(multithreading::Purpose purpose,
                                              CommandPoolFlagBits flags) -> core::ResourceSharedRef;

    // TODO::
    // auto submit();

private:
    using queue_submission_ring_t = std::array<core::ResourceSharedRef, config_t::queue_submission_ring_size_v>;

    using queue_submission_map_t = core::StaticHashTable<queue_submission_ring_t,
                                                         config_t::max_queue_submission_ring_count_v,
                                                         std::underlying_type_t<multithreading::Purpose>,
                                                         uint32_t,
                                                         std::underlying_type_t<gfx::CommandPoolFlags>>;

    using completion_map_t = core::StaticHashTable<uint64_t,
                                                   config_t::max_queue_submission_ring_count_v,
                                                   std::underlying_type_t<multithreading::Purpose>,
                                                   uint32_t,
                                                   std::underlying_type_t<gfx::CommandPoolFlags>>;

    core::ResourceSharedRef deviceRef_;

    queue_submission_map_t queueSubmissionRingMap_;
    completion_map_t completedFrames_;

    uint64_t currentFrameIndex_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_QUEUE_SUBMISSION_MANAGER_H
