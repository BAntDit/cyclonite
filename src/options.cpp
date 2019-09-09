//
// Created by bantdit on 1/19/19.
//

#include "options.h"
#include <iostream>

namespace po = boost::program_options;
namespace pt = boost::property_tree;

namespace cyclonite {
Options::Options(int argc /* = 0*/, const char* argv[] /* = {}*/)
  : config_{ "config.json" }
  , deviceName_{ "" }
  , properties_{}
  , windows_{}
{
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

    if (variables.count("config")) {
        config_ = variables["config"].as<std::string>();
    }

    if (variables.count("device-name")) {
        deviceName_ = variables["device-name"].as<std::string>();
    }

    po::notify(variables);

    if (variables.count("help")) {
        std::cout << commandLineOptions << std::endl;
    }

    {
        std::ifstream ifs(config_);

        if (ifs) {
            pt::read_json(ifs, properties_);
        }
    }

    if (deviceName_.empty() && properties_.count("device")) {
        deviceName_ = properties_.get<std::string>("device");
    }

    parseWindowProperties();

    bool changesInWindowProperties = false;

    if (windows_.empty() && !variables["no-window"].as<bool>()) {
        WindowProperties wp = {};

        wp.title = "Cyclonite";
        wp.left = 0x1FFF0000;
        wp.top = 0x1FFF0000;
        wp.width = variables["width"].as<uint16_t>();
        wp.height = variables["height"].as<uint16_t>();
        wp.fullscreen = variables["full-screen"].as<bool>();

        windows_.push_back(wp);

        changesInWindowProperties = true;
    }

    if (changesInWindowProperties) {
        // todo:: ...
    }
}

void Options::parseWindowProperties()
{
    if (properties_.count("windows") == 0)
        return;

    for (auto const& windowProperties : properties_.get_child("windows")) {
        WindowProperties wp = {};

        wp.title = windowProperties.second.get<std::string>("title");
        wp.left = windowProperties.second.get<uint16_t>("left");
        wp.top = windowProperties.second.get<uint16_t>("top");
        wp.width = windowProperties.second.get<uint16_t>("width");
        wp.height = windowProperties.second.get<uint16_t>("height");
        wp.fullscreen = windowProperties.second.get<bool>("fullscreen");

        windows_.push_back(wp);
    }
}
}
