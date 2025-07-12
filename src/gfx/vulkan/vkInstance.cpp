//
// Created by anton on 6/16/25.
//

#include "vkInstance.h"
#include <iostream>
#include <sstream>
#include <array>
#include <stdexcept>

#if defined(GFX_DRIVER_VULKAN)

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

namespace cyclonite::gfx::vulkan {
namespace {
template<size_t N>
void testLayers(std::array<char const*, N> const& reqLayers)
{
    uint32_t availableInstanceLayerCount = 0;

    if (vkEnumerateInstanceLayerProperties(&availableInstanceLayerCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate available instance layers");
    }

    std::vector<VkLayerProperties> availableLayers{};

    if (availableInstanceLayerCount > 0) {
        availableLayers.resize(availableInstanceLayerCount);

        if (vkEnumerateInstanceLayerProperties(&availableInstanceLayerCount, availableLayers.data()) != VK_SUCCESS) {
            throw std::runtime_error("could not read available instance layer properties");
        }
    }

    for (auto layerIt = reqLayers.cbegin(); layerIt != reqLayers.cend(); ++layerIt) {
        auto it = availableLayers.cbegin();

        while (it != availableLayers.cend()) {
            if (0 == strcmp((*layerIt), (*it).layerName))
                break;
            ++it;
        }

        if (it == availableLayers.cend())
            throw std::runtime_error("required layer: " + std::string(*layerIt) + " is not supported");
    }
}

template<size_t N>
void testExtensions(std::array<char const*, N> const& reqExtensions)
{
    uint32_t availableInstanceExtensionsCount = 0;

    if (vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionsCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate available instance extensions");
    }

    std::vector<VkExtensionProperties> availableExtensions{};

    if (availableInstanceExtensionsCount > 0) {
        availableExtensions.resize(availableInstanceExtensionsCount);

        if (vkEnumerateInstanceExtensionProperties(
              nullptr, &availableInstanceExtensionsCount, availableExtensions.data()) != VK_SUCCESS) {
            throw std::runtime_error("count not read properties of available extensions");
        }
    }

    for (auto extIt = reqExtensions.cbegin(); extIt != reqExtensions.cend(); ++extIt) {
        auto it = availableExtensions.cbegin();

        while (it != availableExtensions.cend()) {
            if (0 == strcmp((*extIt), (*it).extensionName))
                break;
            ++it;
        }

        if (it == availableExtensions.cend()) {
            throw std::runtime_error("required extension: " + std::string(*extIt) + " is not supported");
        }
    }
}
}

Instance::Instance(std::string_view applicationName)
  : physicalDeviceList_{}
  , vkInstance_{ vkDestroyInstance }
  , resourceManager_{}
{
#if !defined(NDEBUG)
    auto reqLayers = std::array<char const*, 1>{ "VK_LAYER_KHRONOS_validation" };
#else
    auto reqLayers = std::array<char const*, 0>{};
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#if !defined(NDEBUG)
    auto reqExtensions = std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                     VK_KHR_XLIB_SURFACE_EXTENSION_NAME };
#else
    auto reqExtensions =
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME };
#endif
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#if !defined(NDEBUG)
    auto reqExtensions = std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                     VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME };
#else
    auto reqExtensions =
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME };
#endif
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#if !defined(NDEBUG)
    auto reqExtensions = std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                     VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#else
    auto reqExtensions =
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#endif
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#if !defined(NDEBUG)
    auto reqExtensions = std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                     VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };
#else
    auto reqExtensions =
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME };
#endif
#else
#if !defined(NDEBUG)
    auto reqExtensions = std::array<char const*, 1>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME };
#else
    auto reqExtensions = std::array<char const*, 0>{};
#endif
#endif

    testLayers(reqLayers);

    testExtensions(reqExtensions);

    auto appInfo = VkApplicationInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = applicationName.data();
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
    instanceInfo.enabledExtensionCount = reqExtensions.size();
    instanceInfo.ppEnabledExtensionNames = reqExtensions.empty() ? nullptr : reqExtensions.data();
    instanceInfo.enabledLayerCount = reqLayers.size();
    instanceInfo.ppEnabledLayerNames = reqLayers.empty() ? nullptr : reqLayers.data();

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

    uint32_t physicalDeviceCount = 0;
    if (vkEnumeratePhysicalDevices(static_cast<VkInstance>(vkInstance_), &physicalDeviceCount, VK_NULL_HANDLE) !=
        VK_SUCCESS) {
        throw std::runtime_error("could not enumerate physical devices");
    }

    physicalDeviceList_.resize(physicalDeviceCount);

    if (vkEnumeratePhysicalDevices(
          static_cast<VkInstance>(vkInstance_), &physicalDeviceCount, physicalDeviceList_.data()) != VK_SUCCESS) {
        throw std::runtime_error("could not get physical devices");
    }
}

auto Instance::chooseBestPhysicalDevice() const -> size_t
{
    // TODO:: implementation required
    return size_t{ 0 };
}

auto Instance::createDevice(uint32_t deviceId /* = std::numeric_limits<uint32_t>::max()*/) -> gfx::ResourceRef
{
    auto result = gfx::ResourceRef{};

    std::vector<const char*> requiredExtensions = {};

#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR) || (VK_USE_PLATFORM_WIN32_KHR) ||        \
  defined(VK_USE_PLATFORM_ANDROID_KHR)
    requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif

    if (deviceId == std::numeric_limits<uint32_t>::max()) {
        deviceId = chooseBestPhysicalDevice();
    }

    for (size_t i = 0, count = physicalDeviceCount(); i < count; i++) {
        auto physicalDevice = physicalDeviceList_[i];
        auto physicalDeviceProperties = VkPhysicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        if (physicalDeviceProperties.deviceID == deviceId) {
            // result = resourceManager_.allocResource<gfx::Device>(
            //  static_cast<VkInstance>(vkInstance_), physicalDevice, physicalDeviceProperties, requiredExtensions);
            break;
        }
    }

    if (!result.valid()) {
        throw std::runtime_error("no suitable physical device to create graphics device over");
    }

    return result;
}
}

#endif