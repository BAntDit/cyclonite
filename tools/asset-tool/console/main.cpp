//
// Created by anton on 8/25/26.
//

#include <boost/program_options.hpp>
#include <cstdint>
#include <iostream>
#include "common.h"

int main(int argc, char* argv[])
{
    namespace po = boost::program_options;

    auto desc = po::options_description{ "asset-tool options", 160 };

    desc.add_options()                                                              // options:
        ("help", "Produce help message")                                            // --help
        ("command", po::value<std::string>(), "command to execute")                 // --command <gltf-to-asset, glb-to-asset>
        ("gltf-src", po::value<std::string>(), "gltf source file");                 // --gltf-src <path>

    auto vm = po::variables_map{};
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help")) {
        std::cout << desc << "\n";
        return 0;
    }

    auto coversionInput = cyclonite::tools::ConversionFromFile{};

    return 0;
}