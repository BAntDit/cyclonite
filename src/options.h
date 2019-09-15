//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_OPTIONS_H
#define CYCLONITE_OPTIONS_H

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace cyclonite {
class Options
{
public:
    struct WindowProperties
    {
        std::string title;
        uint16_t left;
        uint16_t top;
        uint16_t width;
        uint16_t height;
        bool fullscreen;
    };

    explicit Options(int argc = 0, const char* argv[] = {});

    auto deviceName() const -> std::string const& { return deviceName_; }

    void deviceName(std::string const& name) { deviceName_ = name; }

    void save();

private:
    std::string config_;
    std::string deviceName_;
    std::vector<WindowProperties> windows_;
};
}

#endif // CYCLONITE_OPTIONS_H
