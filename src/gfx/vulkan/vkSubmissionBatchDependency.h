
#ifndef CYCLONITE_VK_SUBMISSION_BATCH_DEPENDENCY_H
#define CYCLONITE_VK_SUBMISSION_BATCH_DEPENDENCY_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"

#if defined(GFX_DRIVER_VULKAN)
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class SubmissionBatchDependency
{
public:
    SubmissionBatchDependency(core::ResourceWeakRef signalRef, PipelineStageFlagBits stageMask);

    [[nodiscard]] auto value() const -> uint64_t { return value_; }

    [[nodiscard]] auto signal() const -> core::ResourceWeakRef { return signalRef_; }

    [[nodiscard]] auto stageMask() const -> PipelineStageFlagBits { return stageMask_; }

    [[nodiscard]] auto vulkanStageMask() const -> VkPipelineStageFlags
    {
        return stageMask_.cast_to<VkPipelineStageFlags>();
    }

private:
    core::ResourceWeakRef signalRef_;
    PipelineStageFlagBits stageMask_;
    uint64_t value_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif //  CYCLONITE_VK_SUBMISSION_BATCH_DEPENDENCY_H
