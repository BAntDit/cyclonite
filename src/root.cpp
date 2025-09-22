//
// Created by bantdit on 2/11/20.
//

#include "root.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

namespace cyclonite {
Root::Root()
  : capabilities_{}
  , taskManager_{}
  , gfxInstance_{}
  , input_{}
{
}

void Root::init(std::string_view appName)
{
    // SDL initialization:
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("SDL: could not initialize SDL video subsystem");
    }

    taskManager_.start();

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

    if (!(gfxInstance_ = std::make_unique<gfx::Instance>(appName))) {
        throw std::runtime_error("gfx:: could not create gfx-instance");
    }
}

void Root::reset()
{
    taskManager_.stop();

    SDL_Quit();
}
}
