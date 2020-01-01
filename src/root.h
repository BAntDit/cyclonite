//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "config.h"
#include "input.h"
#include "multithreading/taskManager.h"
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

template<typename EcsConfig>
class Root<Config<EcsConfig>>
{
public:
    struct Capabilities
    {
        std::vector<std::pair<uint16_t, uint16_t>> displayResolutions;
    };

public:
    using config_t = Config<EcsConfig>;

    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    ~Root() = default;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root &&) -> Root& = delete;

    void init(Options const& options);

    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

    [[nodiscard]] auto input() const -> Input const& { return input_; }

    [[nodiscard]] auto input() -> Input& { return input_; }

    [[nodiscard]] auto device() const -> vulkan::Device const& { return *vulkanDevice_; }

    [[nodiscard]] auto device() -> vulkan::Device& { return *vulkanDevice_; }

private:
    Capabilities capabilities_;
    multithreading::TaskManager taskManager_;
    sdl::SDLSupport sdlSupport_;
    std::unique_ptr<vulkan::Instance> vulkanInstance_;
    std::unique_ptr<vulkan::Device> vulkanDevice_;
    Input input_;
};

template<typename EcsConfig>
Root<Config<EcsConfig>>::Root()
  : capabilities_{}
  , taskManager_{}
  , sdlSupport_{}
  , vulkanInstance_{ nullptr }
  , vulkanDevice_{ nullptr }
  , input_{}
{}

template<typename EcsConfig>
void Root<Config<EcsConfig>>::init(Options const& options)
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
#endif // CYCLONITE_ROOT_H
