//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "config.h"
#include "options.h"
#include "sdl/sdlSupport.h"
#include "vulkan/instance.h"
#include <memory>

namespace cyclonite {
template<typename Config>
class Root;

template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
class Root<Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>>
{
public:
    using config_t = Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>;

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
    std::vector<sdl::SDLWindow> windows_;
    std::unique_ptr<vulkan::Instance> vulkanInstance_;
};

template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
Root<Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>>::Root()
  : options_{ nullptr }
  , sdlSupport_{}
  , windows_{}
  , vulkanInstance_{ nullptr }
{}

template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
void Root<Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>>::init(Options const& options)
{
    options_ = std::make_shared<Options>(options);

    int displayIndex = 0; // every time use first display for now

    sdlSupport_.storeDisplayResolutions(*options_, displayIndex);

    options_->adjustWindowResolutions();

    for (auto const& window : options_->windows()) {
        uint32_t flags =
          window.fullscreen ? SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS : SDL_WINDOW_SHOWN;

        windows_.push_back(sdlSupport_.createWindow(window.left, window.top, window.width, window.height, flags));
    }

    if (!windows_.empty()) {
        vulkanInstance_ = std::make_unique<vulkan::Instance>();
    } else {
        vulkanInstance_ =
          std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_LUNARG_standard_validation" },
                                             std::array<char const*, 1>{ VK_EXT_DEBUG_REPORT_EXTENSION_NAME });
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
