
#ifndef CYCLONITE_GFX_VULKAN_EXCEPTION
#define CYCLONITE_GFX_VULKAN_EXCEPTION

#if defined(GFX_DRIVER_VULKAN)

#include <stdexcept>
#include <string_view>
#include <vulkan/vulkan.h>

namespace cyclonite::gfx::vulkan {
class Exception : public std::runtime_error
{
public:
    Exception(VkResult error, std::string_view vkFunctionName);
};
}

#endif // GFX_DRIVER_VULKAN
#endif //  CYCLONITE_GFX_VULKAN_EXCEPTION
