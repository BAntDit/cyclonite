
#include "vkException.h"
#include <cassert>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
auto getErrorName(VkResult error) -> std::string_view
{
    auto result = std::string_view{};

    switch (error) {
        case VK_SUCCESS:
            [[fallthrough]];
        case VK_NOT_READY:
            [[fallthrough]];
        case VK_TIMEOUT:
            [[fallthrough]];
        case VK_EVENT_SET:
            [[fallthrough]];
        case VK_EVENT_RESET:
            [[fallthrough]];
        case VK_INCOMPLETE:
            [[fallthrough]];
        case VK_PIPELINE_COMPILE_REQUIRED:
            assert(false); // non-error core
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            result = "VK_ERROR_OUT_OF_HOST_MEMORY";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            result = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            result = "VK_ERROR_INITIALIZATION_FAILED";
            break;
        case VK_ERROR_DEVICE_LOST:
            result = "VK_ERROR_DEVICE_LOST";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            result = "VK_ERROR_MEMORY_MAP_FAILED";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            result = "VK_ERROR_LAYER_NOT_PRESEN";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            result = "VK_ERROR_EXTENSION_NOT_PRESENT";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            result = "VK_ERROR_FEATURE_NOT_PRESENT";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            result = "VK_ERROR_INCOMPATIBLE_DRIVER";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            result = "VK_ERROR_TOO_MANY_OBJECTS";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            result = "VK_ERROR_FORMAT_NOT_SUPPORTED";
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            result = "VK_ERROR_FRAGMENTED_POOL";
            break;
        case VK_ERROR_UNKNOWN:
            result = "VK_ERROR_UNKNOWN";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            result = "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            result = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_FRAGMENTATION:
            result = "VK_ERROR_FRAGMENTATION";
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            result = "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            result = "VK_ERROR_SURFACE_LOST_KHR";
            break;
        case VK_RESULT_MAX_ENUM:
            [[fallthrough]];
        default:
            result = "Uknown error code";
    }

    return result;
}

auto getErrorDescription(VkResult error) -> std::string_view
{
    auto result = std::string_view{};

    switch (error) {
        case VK_SUCCESS:
            [[fallthrough]];
        case VK_NOT_READY:
            [[fallthrough]];
        case VK_TIMEOUT:
            [[fallthrough]];
        case VK_EVENT_SET:
            [[fallthrough]];
        case VK_EVENT_RESET:
            [[fallthrough]];
        case VK_INCOMPLETE:
            [[fallthrough]];
        case VK_PIPELINE_COMPILE_REQUIRED:
            assert(false); // non-error core
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            result = "A host memory allocation has failed.";
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            result = " A device memory allocation has failed.";
            break;
        case VK_ERROR_INITIALIZATION_FAILED:
            result = " Initialization of an object could not be completed for implementation-specific reasons.";
            break;
        case VK_ERROR_DEVICE_LOST:
            result = "The logical or physical device has been lost. ";
            break;
        case VK_ERROR_MEMORY_MAP_FAILED:
            result = "Mapping of a memory object has failed.";
            break;
        case VK_ERROR_LAYER_NOT_PRESENT:
            result = "A requested layer is not present or could not be loaded.";
            break;
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            result = " A requested extension is not supported.";
            break;
        case VK_ERROR_FEATURE_NOT_PRESENT:
            result = "A requested feature is not supported.";
            break;
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            result = "The requested version of Vulkan is not supported by the driver or is otherwise incompatible for "
                     "implementation-specific reasons";
            break;
        case VK_ERROR_TOO_MANY_OBJECTS:
            result = "Too many objects of the type have already been created.";
            break;
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            result = "A requested format is not supported on this device";
            break;
        case VK_ERROR_FRAGMENTED_POOL:
            result = "A pool allocation has failed due to fragmentation of the pool�s memory.";
            break;
        case VK_ERROR_UNKNOWN:
            result = "An unknown error has occurred; either the application has provided invalid input, or an "
                     "implementation failure has occurred.";
            break;
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            result = "VK_ERROR_OUT_OF_POOL_MEMORY";
            break;
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            result = "VK_ERROR_INVALID_EXTERNAL_HANDLE";
            break;
        case VK_ERROR_FRAGMENTATION:
            result = "A descriptor pool creation has failed due to fragmentation.";
            break;
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            result = " A buffer creation or memory allocation failed because the requested address is not available.";
            break;
        case VK_ERROR_SURFACE_LOST_KHR:
            result = "A surface is no longer available";
            break;
        case VK_RESULT_MAX_ENUM:
            [[fallthrough]];
        default:
            result = "Uknown error code";
    }

    return result;
}

}

Exception::Exception(VkResult error, std::string_view vkFunctionName)
  : std::runtime_error{ std::string("Error: ") + getErrorName(error).data() + " on  attempt to call " +
                        vkFunctionName.data() + ". " + getErrorDescription(error).data() }
{
}
}

#endif // GFX_DRIVER_VULKAN
