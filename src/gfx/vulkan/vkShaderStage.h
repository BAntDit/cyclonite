//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_VK_SHADER_STAGE_H
#define CYCLONITE_VK_SHADER_STAGE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <string>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan
{
class ShaderStage
{
private:
    core::ResourceWeakRef shaderRef_;
    ShaderStageCreationFlagBits creationFlags_;
    ShaderStageFlags stage_;
    std::string entryPointName_;
    // TODO:: specialization info
};
}
#endif //GFX_DRIVER_VULKAN
#endif //CYCLONITE_VK_SHADER_STAGE_H