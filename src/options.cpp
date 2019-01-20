//
// Created by bantdit on 1/19/19.
//

#include "options.h"
#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace cyclonite {
Options::Options(int argc, const char** argv)
  : config_{ "config.json" }
{
    po::options_description commandLineOptions{ "commandLineOptions" };

    commandLineOptions.add_options()("help, h", "show help")(
      "config", po::value<std::string>()->default_value(config_), "config file");

    po::variables_map variables;

    if (argc > 0) {
        po::store(po::parse_command_line(argc, argv, commandLineOptions), variables);
    }

    if (variables.count("config")) {
        config_ = variables["config"].as<std::string>();
    }

    po::notify(variables);
}
}
