//
// Created by bantdit on 9/1/19.
//

#include "instance.h"

namespace cyclonite::vulkan {
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

template<size_t N>
constexpr auto getExtensionNamesArray(std::array<const char*, N>&& list) -> decltype(list) {
    return std::forward<std::array<const char*, N>>(list);
}

void Instance::createInstance(uint32_t layerCount,
                              char const* const* layerNames,
                              uint32_t extensionCount,
                              char const* const* extensionNames)
{
    VkApplicationInfo appInfo = {};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = PROJECT_NAME;
    appInfo.apiVersion = VK_API_VERSION_1_1;
    appInfo.applicationVersion = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

    VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};

    debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debugReportCallbackCreateInfo.pNext = nullptr;
    debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                          VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                          VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    debugReportCallbackCreateInfo.pfnCallback = dbgCallback;
    debugReportCallbackCreateInfo.pUserData = nullptr;

    VkInstanceCreateInfo instanceInfo = {};

    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.pNext = &debugReportCallbackCreateInfo;
    instanceInfo.enabledExtensionCount = extensionCount;
    instanceInfo.ppEnabledExtensionNames = extensionNames;
    instanceInfo.enabledLayerCount = layerCount;
    instanceInfo.ppEnabledLayerNames = layerNames;

    auto vkResult = vkCreateInstance(&instanceInfo, nullptr, &vkInstance_);

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

Instance::Instance(bool presentationSupport /* = true*/)
  : Instance(
      { "VK_LAYER_LUNARG_standard_validation" },
      presentationSupport
        ? std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME }
        : std::array<char const*, 1>({ VK_EXT_DEBUG_REPORT_EXTENSION_NAME }))
{}
}
