//
// Created by anton on 10/4/25.
//

#ifndef CYCLONITE_GFX_QUEUE_SUBMISSION_H
#define CYCLONITE_GFX_QUEUE_SUBMISSION_H

#include "interfaces/queueSubmissionInterface.h"
#if defined(GFX_DRIVER_VULKAN)
#include "vulkan/vkQueueSubmission.h"
#elif defined(GFX_DRIVER_D3D12)
// TODO:: d3d12 not implemented
#else
// TODO:: null api not implemented
#endif

namespace cyclonite::gfx {
#if defined(GFX_DRIVER_VULKAN)
using QueueSubmission = interfaces::QueueSubmissionInterface<vulkan::QueueSubmission>;
#elif defined(GFX_DRIVER_D3D12)
// TODO:: impl
#else
// TODO:: impl
#endif
}

#endif // CYCLONITE_GFX_QUEUE_SUBMISSION_H