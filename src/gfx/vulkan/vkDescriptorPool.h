
#ifndef CYCLONITE_VK_DESCRIPTOR_POOL
#define CYCLONITE_VK_DESCRIPTOR_POOL

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class DescriptorPool : public core::ResourceBase // internal vk resource
{
public:
    DescriptorPool(core::ResourceManagerBase* resourceManager,
                   core::ResourceId resourceId,
                   core::ResourceSharedRef deviceRef,
                   core::ResourceSharedRef layoutRef,
                   uint32_t maxSets,
                   bool allowIndividualReset);

    [[nodiscard]] auto handle() const -> VkDescriptorPool { return static_cast<VkDescriptorPool>(vkDescriptorPool_); }

private:
    core::ResourceSharedRef layoutRef_;
    Handle<VkDescriptorPool> vkDescriptorPool_;
};
}
#endif
#endif // CYCLONITE_VK_DESCRIPTOR_POOL
