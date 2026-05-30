
#ifndef CYCLONITE_VK_DESCRIPTOR_SET
#define CYCLONITE_VK_DESCRIPTOR_SET

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "gfx/descriptorUpdateData.h"
#include "handle.h"
#include <span>
#include <variant>

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

    void update(std::span<DescriptorWriteData const> updateData);

    void copy(core::ResourceSharedRef const& copyFrom, std::span<DescriptorCopyData const> copyData);

private:
    core::ResourceSharedRef descriptorPool_;
    VkDescriptorSet vkDescriptorSet_;
    uint32_t index_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_DESCRIPTOR_SET
