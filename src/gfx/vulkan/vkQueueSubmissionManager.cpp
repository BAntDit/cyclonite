//
// Created by anton on 10/7/25.
//

#include "vkQueueSubmissionManager.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
QueueSubmissionManager::QueueSubmissionManager(core::ResourceSharedRef deviceRef)
  : deviceRef_{ deviceRef }
  , queueSubmissionRingMap_{ std::make_unique<queue_submission_map_t>() }
  , currentFrameIndex_{ 0 }
{
}

}
#endif
