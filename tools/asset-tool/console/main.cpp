//
// Created by anton on 8/25/26.
//

#include <boost/program_options.hpp>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include "common.h"
#include "assetTool.h"

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

    auto command = cyclonite::tools::AssetToolCommand{};

    auto filepath = std::string{};

    if (vm.contains("gltf-src")) {
        filepath = vm["gltf-src"].as<std::string>();
    }

    if (vm.contains("command")) 
    {
        auto cmd = vm["command"].as<std::string>();
        if (cmd == "gltf-to-asset") {
            if (filepath.empty()) {
                std::cout << "gltf-to-asse command requires path to gltf file! - read help for correct usage" << "\n";
                std::cout << "-----------------------" << "\n";
                std::cout << desc << "\n";
                return 0;
            }

            auto coversionInput = cyclonite::tools::ConversionFromFile{};
            coversionInput.path = filepath;

            command.type = cyclonite::tools::CommandType::GLTF_TO_ASSET;

        } else if (cmd == "glb-to-asset") {
            if (filepath.empty()) {
                std::cout << "glb-to-asse command requires path to glb file! - read help for correct usage" << "\n";
                std::cout << "-----------------------" << "\n";
                std::cout << desc << "\n";
                return 0;
            }

            auto coversionInput = cyclonite::tools::ConversionFromFile{};
            coversionInput.path = filepath;

            command.type = cyclonite::tools::CommandType::GLTF_TO_ASSET;

        }
        else {
            std::cout << "uknown command provided! - read help for correct usage" << "\n";
            std::cout << "-----------------------" << "\n";
            std::cout << desc << "\n";
            return 0;
        }
    }
    else {
        std::cout << "no any command provided! - read help for correct usage" << "\n";
        std::cout << "-----------------------" << "\n";
        std::cout << desc << "\n";
        return 0;
    }

    cyclonite::tools::AssetTool::doCommand(command);

    return 0;
}