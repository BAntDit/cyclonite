//
// Created by bantdit on 2/11/20.
//

#include "root.h"

namespace cyclonite {
Root::Root()
  : capabilities_{}
  , taskManager_{}
  , sdlSupport_{}
  , vulkanInstance_
{
#if defined(VK_USE_PLATFORM_XLIB_KHR)
#if !defined(NDEBUG)
    std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_KHRONOS_validation" },
                                       std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                   VK_KHR_SURFACE_EXTENSION_NAME,
                                                                   VK_KHR_XLIB_SURFACE_EXTENSION_NAME })
#else
    std::make_unique<vulkan::Instance>(
      std::array<char const*, 0>{},
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME })
#endif
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#if !defined(NDEBUG)
    std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                       std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                   VK_KHR_SURFACE_EXTENSION_NAME,
                                                                   VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME })
#else
    std::make_unique<vulkan::Instance>(
      std::array<char const*, 0>{},
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME })
#endif
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#if !defined(NDEBUG)
    std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                       std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                   VK_KHR_SURFACE_EXTENSION_NAME,
                                                                   VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
#else
    std::make_unique<vulkan::Instance>(
      std::array<char const*, 0>{},
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME })
#endif
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#if !defined(NDEBUG)
    std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                       std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                   VK_KHR_SURFACE_EXTENSION_NAME,
                                                                   VK_KHR_ANDROID_SURFACE_EXTENSION_NAME })
#else
    std::make_unique<vulkan::Instance>(
      std::array<char const*, 0>{},
      std::array<char const*, 2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME })
#endif
#else
#if !defined(NDEBUG)
    std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                       std::array<char const*, 1>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME })
#else
    std::make_unique<vulkan::Instance>(std::array<char const*, 0>{}, std::array<char const*, 0>{})
#endif
#endif
}
, physicalDeviceList_{}, physicalDevicePropertiesMap_{}, vulkanDevice_{ nullptr }, input_{}
{
    uint32_t physicalDeviceCount = 0;

    if (vkEnumeratePhysicalDevices(vulkanInstance_->handle(), &physicalDeviceCount, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate physical devices");
    }

    physicalDeviceList_.resize(physicalDeviceCount);

    if (vkEnumeratePhysicalDevices(vulkanInstance_->handle(), &physicalDeviceCount, physicalDeviceList_.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("could not get physical devices");
    }

    for (auto const& physicalDevice : physicalDeviceList_) {
        auto properties = VkPhysicalDeviceProperties{};

        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        physicalDevicePropertiesMap_.emplace(std::string{ properties.deviceName }, properties);
    }
}

void Root::init()
{
    init(getDeviceId());
}

void Root::init(uint32_t deviceId)
{
    {
        // every time use first display at least for now
        // multi display support later, okay?
        int displayIndex = 0;

        sdlSupport_.storeDisplayResolutions(capabilities_.displayResolutions, displayIndex);
    }

    {
        std::vector<const char*> requiredExtensions = {};

#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR) || (VK_USE_PLATFORM_WIN32_KHR) ||        \
  defined(VK_USE_PLATFORM_ANDROID_KHR)
        requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif
        for (size_t i = 0, count = getDeviceCount(); i < count; i++) {
            auto&& [name, properties] = *std::next(physicalDevicePropertiesMap_.cbegin(), i);

            if (properties.deviceID != deviceId)
                continue;

            auto&& physicalDevice = physicalDeviceList_[i];

            try {
                vulkanDevice_ = std::make_unique<vulkan::Device>(
                  taskManager_, vulkanInstance_->handle(), physicalDevice, properties, requiredExtensions);
            } catch (const std::exception& e) {
                std::cout << "device " << name << " skipped, cause of: " << e.what() << std::endl;

                throw;
            }

            break;
        }

        if (!vulkanDevice_) {
            throw std::runtime_error("device: " + std::to_string(deviceId) + " is no longer available.");
        }
    } // end vulkan device creation
}

auto Root::getDeviceId(size_t deviceIndex /* = 0*/) const -> uint32_t
{
    assert(deviceIndex < physicalDevicePropertiesMap_.size());

    auto&& [name, properties] = *std::next(physicalDevicePropertiesMap_.cbegin(), deviceIndex);

    (void)name;

    return properties.deviceID;
}

auto Root::getDeviceId(std::string const& deviceName) const -> uint32_t
{
    assert(physicalDevicePropertiesMap_.count(deviceName) > 0);

    auto&& [name, properties] = *physicalDevicePropertiesMap_.find(deviceName);

    (void)name;

    return properties.deviceID;
}
}
