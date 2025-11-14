
#ifndef CYCLONITE_VK_DESCRIPTOR_SET
#define CYCLONITE_VK_DESCRIPTOR_SET

#include "core/resourceBase.h"
#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include "handle.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class DescriptorPool: public core::ResourceBase
{
public:
private:
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_DESCRIPTOR_SET
