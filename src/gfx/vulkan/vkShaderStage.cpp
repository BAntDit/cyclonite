
#include "vkShaderStage.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
ShaderStage::ShaderStage(core::ResourceSharedRef shaderRef,
                         ShaderStageCreationFlagBits creationFlags,
                         ShaderStageFlags stage,
                         std::string_view entryPointName)
  : shaderRef_{ shaderRef }
  , creationFlags_{ creationFlags }
  , stage_{ stage }
  , entryPointName_{ entryPointName }
  , specializationInfo_{}
{
}
}
#endif
