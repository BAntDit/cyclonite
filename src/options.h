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
        uint32_t left;
        uint32_t top;
        uint32_t width;
        uint32_t height;
        bool fullscreen;
    };

    explicit Options(int argc = 0, const char* argv[] = {});

    Options(Options const&) = default;

    Options(Options&&) = default;

    ~Options() = default;

    auto operator=(Options const&) -> Options& = default;

    auto operator=(Options &&) -> Options& = default;

    [[nodiscard]] auto deviceName() const -> std::string const& { return deviceName_; }

    void deviceName(std::string const& name) { deviceName_ = name; }

    [[nodiscard]] auto deviceId() const -> uint32_t { return deviceId_; }

    void deviceId(uint32_t value) { deviceId_ = value; }

    [[nodiscard]] auto windows() const -> std::vector<WindowProperties> const& { return windows_; }

    auto windows() -> std::vector<WindowProperties>& { return windows_; }

    void save();

private:
    std::string config_;
    std::string deviceName_;
    uint32_t deviceId_;
    std::vector<WindowProperties> windows_;
};
}

#endif // CYCLONITE_OPTIONS_H
