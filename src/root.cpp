//
// Created by bantdit on 2/11/20.
//

#include "root.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

namespace cyclonite {
Root::Root()
  : capabilities_{}
  , vulkanInstance_{} /*, vulkanInstance_ // TODO:: move to initialization after refactoring
                    {
                    #if defined(VK_USE_PLATFORM_XLIB_KHR)
                    #if !defined(NDEBUG)
                        std::make_unique<vulkan::Instance>(std::array<char const*, 1>{ "VK_LAYER_KHRONOS_validation" },
                                                           std::array<char const*, 3>{
                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
                                                                                       VK_KHR_XLIB_SURFACE_EXTENSION_NAME
                    }) #else std::make_unique<vulkan::Instance>( std::array<char const*, 0>{}, std::array<char const*,
                    2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_XLIB_SURFACE_EXTENSION_NAME }) #endif #elif
                    defined(VK_USE_PLATFORM_WAYLAND_KHR) #if !defined(NDEBUG)
                        std::make_unique<vulkan::Instance>(std::array<char const*, 1>{
                    "VK_LAYER_LUNARG_standard_validation" }, std::array<char const*, 3>{
                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
                                                                                       VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME
                    }) #else std::make_unique<vulkan::Instance>( std::array<char const*, 0>{}, std::array<char const*,
                    2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME }) #endif #elif
                    defined(VK_USE_PLATFORM_WIN32_KHR) #if !defined(NDEBUG)
                        std::make_unique<vulkan::Instance>(std::array<char const*, 1>{
                    "VK_LAYER_LUNARG_standard_validation" }, std::array<char const*, 3>{
                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
                                                                                       VK_KHR_WIN32_SURFACE_EXTENSION_NAME
                    }) #else std::make_unique<vulkan::Instance>( std::array<char const*, 0>{}, std::array<char const*,
                    2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME }) #endif #elif
                    defined(VK_USE_PLATFORM_ANDROID_KHR) #if !defined(NDEBUG)
                        std::make_unique<vulkan::Instance>(std::array<char const*, 1>{
                    "VK_LAYER_LUNARG_standard_validation" }, std::array<char const*, 3>{
                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_KHR_SURFACE_EXTENSION_NAME,
                                                                                       VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
                    }) #else std::make_unique<vulkan::Instance>( std::array<char const*, 0>{}, std::array<char const*,
                    2>{ VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME }) #endif #else #if
                    !defined(NDEBUG) std::make_unique<vulkan::Instance>(std::array<char const*, 1>{
                    "VK_LAYER_LUNARG_standard_validation" }, std::array<char const*, 1>{
                    VK_EXT_DEBUG_REPORT_EXTENSION_NAME }) #else std::make_unique<vulkan::Instance>(std::array<char
                    const*, 0>{}, std::array<char const*, 0>{}) #endif #endif
                    }*/
  , input_{}
{
}

void Root::init()
{
    init(0);
}

void Root::init(uint32_t deviceId)
{
    // SDL initialization:
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("SDL: could not initialize SDL video subsystem");
    }

    {
        auto const displayId = SDL_GetPrimaryDisplay();
        auto displayModeCount = int32_t{ 0 };

        if (auto** displayModes = SDL_GetFullscreenDisplayModes(displayId, &displayModeCount);
            displayModes != nullptr && displayModeCount > 0) {
            auto displayResolutions = std::vector<std::pair<uint16_t, uint16_t>>{};
            displayResolutions.reserve(displayModeCount);

            for (auto i = 0; i < displayModeCount; i++) {
                auto const& displayMode = *(displayModes[i]);

                auto width = static_cast<uint16_t>(displayMode.w);
                auto height = static_cast<uint16_t>(displayMode.h);

                displayResolutions.emplace_back(width, height);
            }

            std::swap(displayResolutions, capabilities_.displayResolutions);
        } else {
            throw std::runtime_error("SDL: could not get available display modes");
        }
    }
}

void Root::reset()
{
    SDL_Quit();
}
}
