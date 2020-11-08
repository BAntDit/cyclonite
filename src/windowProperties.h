//
// Created by bantdit on 11/3/20.
//

#ifndef CYCLONITE_WINDOWPROPERTIES_H
#define CYCLONITE_WINDOWPROPERTIES_H

#include <cstdint>
#include <string>

namespace cyclonite {
struct WindowProperties
{
    WindowProperties() noexcept;

    WindowProperties(std::string const& title,
                     uint32_t left,
                     uint32_t top,
                     uint32_t width,
                     uint32_t height,
                     bool fullscreen) noexcept;

    std::string title;
    uint32_t left;
    uint32_t top;
    uint32_t width;
    uint32_t height;
    bool fullscreen;
};
}

#endif // CYCLONITE_WINDOWPROPERTIES_H
