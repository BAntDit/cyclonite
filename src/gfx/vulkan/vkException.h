
#ifndef CYCLONITE_GFX_VULKAN_EXCEPTION
#define CYCLONITE_GFX_VULKAN_EXCEPTION

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <string_view>

namespace cyclonite::gfx::vulkan {
class Exception: public std::runtime_error
{
public:
    Exception(VkResult error, std::string_view vkFunctionName);
};
}

#endif //  CYCLONITE_GFX_VULKAN_EXCEPTION
