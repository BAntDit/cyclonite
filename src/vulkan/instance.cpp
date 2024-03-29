//
// Created by bantdit on 9/1/19.
//

#include "instance.h"

namespace cyclonite::vulkan {
#if !defined(NDEBUG)
static VKAPI_ATTR VkBool32 VKAPI_CALL dbgCallback(VkFlags msgFlags,
                                                  VkDebugReportObjectTypeEXT objectTypeEXT,
                                                  uint64_t object,
                                                  size_t location,
                                                  int32_t msgCode,
                                                  const char* pLayerPrefix,
                                                  const char* pMsg,
                                                  void* pUserData)
{
    (void)objectTypeEXT;
    (void)object;
    (void)location;
    (void)pUserData;

    std::string message = pMsg;
    std::ostringstream oss;

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        oss << "ERROR: " << pLayerPrefix << " Code " << msgCode << ": " << message.c_str();
        std::cout << oss.str() << std::endl;
    } else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
        oss << "WARNING: " << pLayerPrefix << " Code " << msgCode << ": " << message.c_str();
    } else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        oss << "PERFORMANCE WARNING: " << pLayerPrefix << " Code " << msgCode << ": " << message.c_str();
    } else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        oss << "INFO: " << pLayerPrefix << " Code " << msgCode << ": " << message.c_str();
    } else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        oss << "DEBUG: " << pLayerPrefix << " Code " << msgCode << ": " << message.c_str();
    }

    std::cout << oss.str() << std::endl;

    /*
     * false indicates that layer should not bail-out of an
     * API call that had validation failures. This may mean that the
     * app dies inside the driver due to invalid parameter(s).
     * That's what would happen without validation layers, so we'll
     * keep that behavior here.
     */
    return VK_FALSE;
}
#endif

template<size_t N>
constexpr auto getExtensionNamesArray(std::array<const char*, N>&& list) -> decltype(list)
{
    return std::forward<std::array<const char*, N>>(list);
}

void Instance::createInstance(uint32_t layerCount,
                              char const* const* layerNames,
                              uint32_t extensionCount,
                              char const* const* extensionNames)
{
    auto appInfo = VkApplicationInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = PROJECT_NAME;
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

#if !defined(NDEBUG)
    auto debugReportCallbackCreateInfo = VkDebugReportCallbackCreateInfoEXT{};

    debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugReportCallbackCreateInfo.pNext = nullptr;
    debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                          VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                          VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    debugReportCallbackCreateInfo.pfnCallback = dbgCallback;
    debugReportCallbackCreateInfo.pUserData = nullptr;
#endif

    auto instanceInfo = VkInstanceCreateInfo{};

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
#if !defined(NDEBUG)
    instanceInfo.pNext = &debugReportCallbackCreateInfo;
#endif
    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = extensionNames;
    instanceInfo.enabledLayerCount = layerCount;
    instanceInfo.ppEnabledLayerNames = layerNames;

    if (auto vkResult = vkCreateInstance(&instanceInfo, nullptr, &vkInstance_); vkResult != VK_SUCCESS) {
        switch (vkResult) {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                throw std::runtime_error("system is running out of memory");
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                throw std::runtime_error("device is running out of memory");
            case VK_ERROR_LAYER_NOT_PRESENT:
                throw std::runtime_error("layer is not presented");
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                throw std::runtime_error("extension is not presented");
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                throw std::runtime_error("incompatible driver");
            default:
                assert(false);
        }

        throw std::runtime_error("vulkan instance creation failed"); // and no one knows why
    }
}
}
