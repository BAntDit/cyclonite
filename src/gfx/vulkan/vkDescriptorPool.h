
#ifndef CYCLONITE_VK_DESCRIPTOR_POOL
#define CYCLONITE_VK_DESCRIPTOR_POOL

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"
#include <atomic>

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

    [[nodiscard]] auto allocateDescriptorSet() -> VkDescriptorSet;

    [[nodiscard]] auto unused() const -> bool;

    void freeDescriptorSet(VkDescriptorSet vkDescriptorSet);

private:
    core::ResourceSharedRef deviceRef_;
    core::ResourceSharedRef layoutRef_;
    Handle<VkDescriptorPool> vkDescriptorPool_;
    uint32_t maxSets_;
    std::atomic<uint32_t> allocationCount_;
    std::atomic<uint32_t> deallocationCount_;
    VkDescriptorPoolCreateFlags flags_;
};

}
#endif
#endif // CYCLONITE_VK_DESCRIPTOR_POOL
