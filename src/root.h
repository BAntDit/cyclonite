//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "config.h"
#include "options.h"
#include "platform.h"
#include "sdl/sdlSupport.h"
#include "surface.h"
#include "vulkan/device.h"
#include "vulkan/instance.h"

#include <iostream>
#include <memory>

namespace cyclonite {
template<typename Config>
class Root;

template<typename PlatformConfig, typename EcsConfig>
class Root<Config<PlatformConfig, EcsConfig>>
{
public:
    using config_t = Config<PlatformConfig, EcsConfig>;

    using window_surface_t = Surface<typename PlatformConfig::surface_type_t>;

    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    ~Root() = default;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root &&) -> Root& = delete;

    void init(Options const& options);

private:
    std::shared_ptr<Options> options_;
    sdl::SDLSupport sdlSupport_;
    std::unique_ptr<vulkan::Instance> vulkanInstance_;
    std::vector<window_surface_t> surfaces_;
    std::unique_ptr<vulkan::Device> vulkanDevice_;
};

template<typename PlatformConfig, typename EcsConfig>
Root<Config<PlatformConfig, EcsConfig>>::Root()
  : options_{ nullptr }
  , sdlSupport_{}
  , vulkanInstance_{ nullptr }
  , surfaces_{}
  , vulkanDevice_{ nullptr }
{}

template<typename PlatformConfig, typename EcsConfig>
void Root<Config<PlatformConfig, EcsConfig>>::init(Options const& options)
{
    options_ = std::make_shared<Options>(options);

    int displayIndex = 0; // every time use first display for now

    sdlSupport_.storeDisplayResolutions(*options_, displayIndex);

    options_->adjustWindowResolutions();

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

    surfaces_.reserve(options_->windows().size());

    for (auto const& window : options_->windows()) {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        surfaces_.emplace_back(vulkanInstance_->handle(), window, easy_mp::type_list<Display*, Window const&>{});
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        surfaces_.emplace_back(vulkanInstance_->handle(), window, easy_mp::type_list<wl_display*, wl_surface*>{});
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        surfaces_.emplace_back(vulkanInstance_->handle(), window, easy_mp::type_list<HINSTANCE, HWND>{});
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        surfaces_.emplace_back(vulkanInstance_->handle(), window, easy_mp::type_list<ANativeWindow*>{});
#endif
    }

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
        auto testSurfacesSupport = [&](VkPhysicalDevice const& physicalDevice, uint32_t queueFamilyIndex) -> bool {
            bool presentationSupportResult = true;

            for (auto const& surface : surfaces_) {
                VkBool32 presentationSupport = VK_FALSE;

                if (auto result = vkGetPhysicalDeviceSurfaceSupportKHR(
                      physicalDevice, queueFamilyIndex, surface.handle(), &presentationSupport);
                    result != VK_SUCCESS) {
                    if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
                        throw std::runtime_error("device has no enough memory to test surface support");
                    }

                    if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
                        throw std::runtime_error("device has no enough GPU memory to test surface support");
                    }

                    if (result == VK_ERROR_SURFACE_LOST_KHR) {
                        throw std::runtime_error("surface lost");
                    }

                    assert(false);

                    std::terminate();
                }

                if (presentationSupport == VK_FALSE) {
                    presentationSupportResult = false;
                    break;
                }
            }

            return presentationSupportResult;
        };

        std::vector<const char*> requiredExtensions = {};

        if (!surfaces_.empty()) {
            requiredExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        }

        for (auto const& physicalDevice : physicalDeviceList) {
            VkPhysicalDeviceProperties properties = {};

            vkGetPhysicalDeviceProperties(physicalDevice, &properties);

            if (options_->deviceId() != std::numeric_limits<uint32_t>::max() &&
                options_->deviceId() == properties.deviceID) {

                try {
                    auto vulkanDevice =
                      std::make_unique<vulkan::Device>(physicalDevice, properties, requiredExtensions);

                    if (!testSurfacesSupport(physicalDevice, vulkanDevice_->graphicsQueueFamilyIndex())) {
                        throw std::runtime_error("device graphics queue does not support surface");
                    }

                    std::swap(vulkanDevice_, vulkanDevice);
                } catch (const std::exception& e) {
                    std::cout << "device " << properties.deviceName << " skipped, cause of: " << e.what() << std::endl;
                }

                break;
            } else if (options_->deviceId() == std::numeric_limits<uint32_t>::max() &&
                       options_->deviceName() == properties.deviceName) {
                try {
                    auto vulkanDevice =
                      std::make_unique<vulkan::Device>(physicalDevice, properties, requiredExtensions);

                    if (!testSurfacesSupport(physicalDevice, vulkanDevice_->graphicsQueueFamilyIndex())) {
                        throw std::runtime_error("device graphics queue does not support surface");
                    }

                    std::swap(vulkanDevice_, vulkanDevice);

                    options_->deviceId(properties.deviceID);
                } catch (std::exception const& e) {
                    std::cout << "device " << properties.deviceName << " skipped, cause of: " << e.what() << std::endl;
                }

                break;
            }
        }

        if (!vulkanDevice_) {
            std::cout << "device: " << options_->deviceName() << ", id: (" << options_->deviceId() << ") "
                      << " is no longer available.";

            for (auto const& physicalDevice : physicalDeviceList) {
                VkPhysicalDeviceProperties properties = {};

                vkGetPhysicalDeviceProperties(physicalDevice, &properties);

                try {
                    auto vulkanDevice =
                      std::make_unique<vulkan::Device>(physicalDevice, properties, requiredExtensions);

                    if (!testSurfacesSupport(physicalDevice, vulkanDevice_->graphicsQueueFamilyIndex())) {
                        throw std::runtime_error("device graphics queue does not support surface");
                    }

                    std::swap(vulkanDevice_, vulkanDevice);

                    options_->deviceName(properties.deviceName);
                    options_->deviceId(properties.deviceID);

                    std::cout << "device:" << properties.deviceName << ", id: (" << options_->deviceId() << ") "
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

    options_->save();
}
}
#endif // CYCLONITE_ROOT_H
