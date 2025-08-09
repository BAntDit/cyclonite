//
// Created by anton on 7/23/25.
//

#include "vkSurface.h"
#include "core/resourceRef.h"
#include "gfx/resourceManager.h"
#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>

#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include "vkSurfaceXlib.h"
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
#include "vkSurfaceWayland.h"
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#include "vkSurfaceWin32.h"
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
#include "vkSurfaceAndroid.h"
#else
static_assert(false, "wrong platform configuration, check value of -o platform=<value> this project installed with.");
#endif

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
template<typename T>
auto getWindowProperty(SDL_Window* window) -> T;

#if defined(VK_USE_PLATFORM_XLIB_KHR)
template<>
auto getWindowProperty<Display*>(SDL_Window* window) -> Display*
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0);
    return static_cast<Display*>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
}

template<>
auto getWindowProperty<Window>(SDL_Window* window) -> Window
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0);
    return static_cast<Window>(
      SDL_GetNumberProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
template<>
auto getWindowProperty<wl_display*>(SDL_Window* window) -> wl_display*
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0);
    return static_cast<wl_display*>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
}

template<>
auto getWindowProperty<wl_surface*>(SDL_Window* window) -> wl_surface*
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "wayland") == 0);
    return static_cast<wl_surface*>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));
}
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
template<>
auto getWindowProperty<ANativeWindow*>(SDL_Window* window) -> ANativeWindow*
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "android") == 0);
    return static_cast<ANativeWindow*>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, nullptr))
}
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
template<>
auto getWindowProperty<HWND>(SDL_Window* window) -> HWND
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "windows") == 0);
    return static_cast<HWND>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
}

template<>
auto getWindowProperty<HINSTANCE>(SDL_Window* window) -> HINSTANCE
{
    assert(window != nullptr);
    assert(SDL_strcmp(SDL_GetCurrentVideoDriver(), "windows") == 0);
    return static_cast<HINSTANCE>(
      SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, nullptr));
}
#endif

template<typename... SurfaceArgs>
auto createPlatformSurface(VkInstance vkInstance, SDL_Window* sdlWindow, metrix::type_list<SurfaceArgs...>)
  -> platform_surface_t*
{
    return new platform_surface_t{ vkInstance, getWindowProperty<SurfaceArgs>(sdlWindow)... };
}
}
Surface::Surface(core::ResourceManagerBase* resourceManager,
                 core::ResourceId resourceId,
                 core::ResourceRef deviceRef,
                 uint32_t width,
                 uint32_t height,
                 std::string_view title,
                 SurfaceFlagBits flags)
  : core::ResourceBase{ resourceManager, resourceId, true }
  , extent_{}
  , sdlWindowPtr_{ SDL_CreateWindow(title.data(),
                                    static_cast<int>(width),
                                    static_cast<int>(height),
                                    flags.cast_to<SDL_WindowFlags>()),
                   [](SDL_Window* window) { SDL_DestroyWindow(window); } }
  , surface_{ createPlatformSurface(
      deviceRef.as<gfx::type_traits::platform_implementation_t<gfx::Device>>().vulkanInstance(),
      sdlWindowPtr_.get(),
      platform_surface_argument_type_list_t{}) }
{
}
}
#endif