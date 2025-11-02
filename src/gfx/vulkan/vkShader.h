//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_VK_SHADER_H
#define CYCLONITE_VK_SHADER_H

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"

namespace cyclonite::gfx::vulkan {
class Shader : public core::ResourceBase
{
public:
    Shader(core::ResourceManagerBase* resourceManager,
           core::ResourceId resourceId,
           core::ResourceSharedRef deviceRef,
           size_t codeWordCount,
           uint32_t const* codeWords);

    [[nodiscard]] auto id() const -> uint32_t { return id_; }

    [[nodiscard]] auto handle() const -> VkShaderModule { return static_cast<VkShaderModule>(vkShaderModule_); }

private:
    uint32_t id_;
    Handle<VkShaderModule> vkShaderModule_;
};
}

#endif // CYCLONITE_VK_SHADER_H