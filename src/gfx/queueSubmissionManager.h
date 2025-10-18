//
// Created by anton on 10/18/25.
//

#ifndef CYCLONITE_QUEUE_SUBMISSION_MANAGER_H
#define CYCLONITE_QUEUE_SUBMISSION_MANAGER_H

#include "interfaces/queueSubmissionManagerInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkQueueSubmissionManager.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using QueueSubmissionManager = interfaces::QueueSubmissionManagerInterface<vulkan::QueueSubmissionManager>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_QUEUE_SUBMISSION_MANAGER_H