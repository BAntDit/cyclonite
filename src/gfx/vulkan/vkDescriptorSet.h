
#ifndef CYCLONITE_VK_DESCRIPTOR_SET
#define CYCLONITE_VK_DESCRIPTOR_SET

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class DescriptorSet : public core::ResourceBase
{
public:
    DescriptorSet(core::ResourceManagerBase* resourceManager,
                  core::ResourceId resourceId,
                  core::ResourceSharedRef descriptorPool,
                  uint32_t setIndex);

    ~DescriptorSet();

    [[nodiscard]] auto index() const -> uint32_t { return index_; }

    [[nodiscard]] auto handle() const -> VkDescriptorSet { return vkDescriptorSet_; }

private:
    core::ResourceSharedRef descriptorPool_;
    VkDescriptorSet vkDescriptorSet_;
    uint32_t index_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_DESCRIPTOR_SET
