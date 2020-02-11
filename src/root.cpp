//
// Created by bantdit on 2/11/20.
//

#include "root.h"

namespace cyclonite {
Root::Root()
  : capabilities_{}
  , taskManager_{}
  , sdlSupport_{}
  , vulkanInstance_{ nullptr }
  , vulkanDevice_{ nullptr }
  , input_{}
{}

void Root::init(Options const& options)
{
    int displayIndex = 0; // every time use first display for now

    sdlSupport_.storeDisplayResolutions(capabilities_.displayResolutions, displayIndex);

    vulkanInstance_ =
#if defined(VK_USE_PLATFORM_XLIB_KHR)
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                                     VK_KHR_XLIB_SURFACE_EXTENSION_NAME });
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                                     VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME });
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                                     VK_KHR_WIN32_SURFACE_EXTENSION_NAME });
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                                     VK_KHR_ANDROID_SURFACE_EXTENSION_NAME });
#else
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 1>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME });
#endif

    uint32_t physicalDeviceCount = 0;

    if (vkEnumeratePhysicalDevices(vulkanInstance_->handle(), &physicalDeviceCount, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate physical devices");
    }

    std::vector<VkPhysicalDevice> physicalDeviceList(physicalDeviceCount);

    if (vkEnumeratePhysicalDevices(vulkanInstance_->handle(), &physicalDeviceCount, physicalDeviceList.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("could not get physical devices");
    }

    {
        std::vector<const char*> requiredExtensions = {};

#if defined(VK_USE_PLATFORM_XLIB_KHR) || defined(VK_USE_PLATFORM_WAYLAND_KHR) || (VK_USE_PLATFORM_WIN32_KHR) ||        \
  defined(VK_USE_PLATFORM_ANDROID_KHR)
        requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
#endif

        for (auto const& physicalDevice : physicalDeviceList) {
            VkPhysicalDeviceProperties properties = {};

            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            if (options.deviceId() != std::numeric_limits<uint32_t>::max() &&
                options.deviceId() == properties.deviceID) {

                try {
                    vulkanDevice_ = std::make_unique<vulkan::Device>(
                      taskManager_, vulkanInstance_->handle(), physicalDevice, properties, requiredExtensions);
                } catch (const std::exception& e) {
                    std::cout << "device " << properties.deviceName << " skipped, cause of: " << e.what() << std::endl;
                }

                break;
            } else if (options.deviceId() == std::numeric_limits<uint32_t>::max() &&
                       options.deviceName() == properties.deviceName) {
                try {
                    vulkanDevice_ = std::make_unique<vulkan::Device>(
                      taskManager_, vulkanInstance_->handle(), physicalDevice, properties, requiredExtensions);
                } catch (std::exception const& e) {
                    std::cout << "device " << properties.deviceName << " skipped, cause of: " << e.what() << std::endl;
                }

                break;
            }
        }

        if (!vulkanDevice_) {
            std::cout << "device: " << options.deviceName() << ", id: (" << options.deviceId() << ") "
                      << " is no longer available.";

            for (auto const& physicalDevice : physicalDeviceList) {
                VkPhysicalDeviceProperties properties = {};

                vkGetPhysicalDeviceProperties(physicalDevice, &properties);

                try {
                    vulkanDevice_ = std::make_unique<vulkan::Device>(
                      taskManager_, vulkanInstance_->handle(), physicalDevice, properties, requiredExtensions);

                    std::cout << "device:" << properties.deviceName << ", id: (" << options.deviceId() << ") "
                              << " will set as device." << std::endl;

                    break;
                } catch (std::exception const& e) {
                    std::cout << "device " << properties.deviceName << " skipped, cause of: " << e.what() << std::endl;
                }
            }
        }

        if (!vulkanDevice_) {
            throw std::runtime_error("there is no suitable physical device on the host");
        }
    } // end vulkan device creation
}
}
