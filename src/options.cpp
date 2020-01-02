//
// Created by bantdit on 1/19/19.
//

#include "options.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;

namespace cyclonite {
static auto parseWindowProperties(nlohmann::json const& json) -> Options::WindowProperties
{
    Options::WindowProperties wp{};

    {
        auto it = json.find(u8"title");

        if (it != json.end() && (*it).is_string()) {
            wp.title = (*it).get<std::string>();
        } else {
            wp.title = "";
        }
    }

    {
        auto it = json.find(u8"left");

        if (it != json.end() && (*it).is_number()) {
            wp.left = (*it).get<uint16_t>();
        }
    }

    {
        auto it = json.find(u8"top");

        if (it != json.end() && (*it).is_number()) {
            wp.top = (*it).get<uint16_t>();
        }
    }

    {
        auto it = json.find(u8"width");

        if (it != json.end() && (*it).is_number()) {
            wp.width = (*it).get<uint16_t>();
        }
    }

    {
        auto it = json.find(u8"height");

        if (it != json.end() && (*it).is_number()) {
            wp.height = (*it).get<uint16_t>();
        }
    }

    {
        auto it = json.find(u8"fullscreen");

        if (it != json.end() && (*it).is_boolean()) {
            wp.fullscreen = (*it).get<bool>();
        }
    }

    return wp;
}

Options::Options(int argc /* = 0*/, const char* argv[] /* = {}*/)
  : config_{ "config.json" }
  , deviceName_{ "" }
  , deviceId_{ std::numeric_limits<uint32_t>::max() }
  , windows_{}
{
    // TODO:: move options parsing outside the engine
    po::options_description commandLineOptions{ "commandLineOptions" };

    commandLineOptions.add_options()("help, h", "show help")(
      "config", po::value<std::string>()->default_value(config_), "config file")(
      "device-name", po::value<std::string>(), "device name to use")(
      "no-window", po::value<bool>()->default_value(false), "true to prevent default window creation")(
      "full-screen",
      po::value<bool>()->default_value(false),
      "whether default window is in full screen mode (true / false)")(
      "width", po::value<uint16_t>()->default_value(512), "default window screen width")(
      "height", po::value<uint16_t>()->default_value(512), "default window screen height");

    po::variables_map variables;

    if (argc > 0) {
        po::store(po::parse_command_line(argc, argv, commandLineOptions), variables);
    }

    if (variables.count("config") > 0) {
        config_ = variables["config"].as<std::string>();
    }

    if (variables.count("device-name") > 0) {
        deviceName_ = variables["device-name"].as<std::string>();
    }

    po::notify(variables);

    if (variables.count("help") > 0) {
        std::cout << commandLineOptions << std::endl;
    }

    nlohmann::json json;

    {
        std::ifstream ifs(config_);

        if (ifs) {
            ifs >> json;
        }
    }

    if (deviceName_.empty()) {
        {
            auto it = json.find(u8"device-name");

            if (it != json.end()) {
                deviceName_ = it.value().get<std::string>();
            }
        }

        {
            auto it = json.find(u8"device-id");

            if (it != json.end()) {
                deviceId_ = it.value().get<uint32_t>();
            }
        }
    }

    {
        auto it = json.find(u8"windows");

        if (it != json.end()) {
            auto const& windows = *it;

            if (windows.is_array()) {
                for (size_t i = 0; i < windows.size(); i++) {
                    windows_.push_back(parseWindowProperties(windows.at(i)));
                }
            }
        }
    }

    if (windows_.empty() && !variables["no-window"].as<bool>()) {
        WindowProperties defaultWindow{};

        defaultWindow.title = "Cyclonite";
        defaultWindow.left = 0x1FFF;
        defaultWindow.top = 0x1FFF;
        defaultWindow.height = variables["height"].as<uint16_t>();
        defaultWindow.width = variables["width"].as<uint16_t>();
        defaultWindow.fullscreen = variables["full-screen"].as<bool>();
    }
}

void Options::save()
{
    nlohmann::json json = { { u8"device-name", deviceName_ }, { u8"device-id", deviceId_ } };

    auto windows = nlohmann::json::array();

    for (auto const& [title, top, left, width, height, fullscreen] : windows_) {
        nlohmann::json wp = { { u8"title", title }, { u8"top", top },       { u8"left", left },
                              { u8"width", width }, { u8"height", height }, { u8"fullscreen", fullscreen } };

        windows.push_back(wp);
    }

    json[u8"windows"] = windows;

    std::ofstream ofs(config_, std::ofstream::trunc);

    ofs << std::setw(4) << json << std::endl;
}
}
