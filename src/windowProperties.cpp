//
// Created by bantdit on 11/3/20.
//

#include "windowProperties.h"

namespace cyclonite {
WindowProperties::WindowProperties(std::string const& title,
                                   uint32_t left,
                                   uint32_t top,
                                   uint32_t width,
                                   uint32_t height,
                                   bool fullscreen) noexcept
  : title{ title }
  , left{ left }
  , top{ top }
  , width{ width }
  , height{ height }
  , fullscreen{ fullscreen }
{}

WindowProperties::WindowProperties() noexcept
  : WindowProperties{ "", 0, 0, 512, 512, false }
{}
}