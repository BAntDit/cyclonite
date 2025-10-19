//
// Created by bantdit on 2/11/20.
//

#include "root.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>

namespace cyclonite {
Root::Root()
  : capabilities_{}
  , taskManager_{ nullptr }
  , gfxInstance_{ nullptr }
  , input_{}
{
}

void Root::init(std::string_view appName)
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

    if (!(gfxInstance_ = std::make_unique<gfx::Instance>(appName))) {
        throw std::runtime_error("gfx:: could not create gfx-instance");
    }
}

void Root::initTaskManager(bool dedicatedTransferRequired,
                           bool dedicatedComputeRequired,
                           size_t threadPoolSize /* = std::max(std::thread::hardware_concurrency(), 1u)*/)
{
    if (taskManager_ = std::make_unique<multithreading::TaskManager>(
          dedicatedTransferRequired, dedicatedComputeRequired, threadPoolSize);
        taskManager_) {
        taskManager_->start();
        if (auto ex = taskManager_->getLastException()) {
            std::rethrow_exception(ex);
        }
    } else {
        throw std::runtime_error("Root:: could not initialize task manager");
    }
}

void Root::reset()
{
    if (taskManager_) {
        taskManager_->stop();
        taskManager_.reset();
    }

    SDL_Quit();
}
}
