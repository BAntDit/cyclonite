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
#include "vulkan/instance.h"

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
};

template<typename SurfaceType, typename EcsConfig>
Root<Config<SurfaceType, EcsConfig>>::Root()
  : options_{ nullptr }
  , sdlSupport_{}
  , vulkanInstance_{ nullptr }
  , surfaces_{}
{}

template<typename SurfaceType, typename EcsConfig>
void Root<Config<SurfaceType, EcsConfig>>::init(Options const& options)
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
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 3>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
                                                                     VK_KHR_SURFACE_EXTENSION_NAME,
                                                                     VK_KHR_WIN32_SURFACE_EXTENSION_NAME });
#else
      std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                         std::array<char const*, 1>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME });
#endif

    surfaces_.reserve(options_->windows().size());

    for (auto const& window : options_->windows()) {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        surfaces_.emplace_back(vulkanInstance_->handle(), window, easy_mp::type_list<Display*, Window const*>{});
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        surfaces_.emplace_back(vulkanInstance_->handle(), window, easy_mp::type_list<HINSTANCE, HWND>{});
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

    options_->save();
}
}
#endif // CYCLONITE_ROOT_H
