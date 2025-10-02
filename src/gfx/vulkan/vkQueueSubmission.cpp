
#include "vkQueueSubmission.h"
#include "gfx/device.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
QueueSubmission::QueueSubmission(core::ResourceSharedRef deviceRef,
                                 uint32_t queueFamilyIndex,
                                 CommandPoolFlagBits commandPoolFlags)
  : commandPool_{}
  , batches_{}
  , competitionValue_{}
  , commonListsState_{ CommandListState::Initial }
{
    assert(deviceRef.valid());
    auto& device = deviceRef.as<type_traits::platform_implementation_t<gfx::Device>>();

    commandPool_ = device.createCommandPool(queueFamilyIndex, commandPoolFlags);
}
}
#endif
