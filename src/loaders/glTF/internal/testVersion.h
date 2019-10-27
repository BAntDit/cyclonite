//
// Created by bantdit on 10/26/19.
//

#ifndef CYCLONITE_GLTF_LOADER_INTERNAL_TESTVERSION_H
#define CYCLONITE_GLTF_LOADER_INTERNAL_TESTVERSION_H

#include <nlohmann/json.hpp>
#include <regex>

namespace cyclonite::loaders::gltf::internal {
static auto testVersion(nlohmann::json& asset) -> bool
{
    // From spec: https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#asset
    //
    // Implementation Note: Client implementations should first check whether a minVersion property is specified
    // and ensure both major and minor versions can be supported.
    // If no minVersion is specified, then clients should check the version property
    // and ensure the major version is supported.

    uint16_t minimalMajorVersion = 2;
    uint16_t supportedMajorVersion = 2;
    uint16_t supportedMinorVersion = 0;

    // check minVersion at first
    {
        // it must contains minor and major version at once, rest part of version is optional
        std::regex re(u8"^(\\d+).(\\d+)($|.\\d+$|(.\\d+.\\d+$))");
        std::smatch matches;

        auto it = asset.find(u8"minVersion");

        if (it != asset.end()) {
            if (!it.value().is_string()) {
                throw std::runtime_error("glTF asset has wrong the minVersion value type");
            }

            auto minVersion = it.value().get<std::string>();

            if (std::regex_match(minVersion, matches, re)) {
                assert(matches.size() >= 2);

                auto majorVersion = static_cast<uint16_t>(strtol(matches[0].str().c_str(), nullptr, 10));
                auto minorVersion = static_cast<uint16_t>(strtol(matches[1].str().c_str(), nullptr, 10));

                if (supportedMajorVersion >= majorVersion && majorVersion >= minimalMajorVersion) {
                    return !(supportedMajorVersion == majorVersion && supportedMinorVersion < minorVersion);
                }

                return false;
            }

            throw std::runtime_error("glTF asset has wrong the minVersion format");
        }
    }

    // check version
    {
        auto it = asset.find(u8"version");

        // if there is no minVersion, the target version must be
        // From spec: https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#asset
        // The asset object must contain glTF version which specifies the target glTF version of the asset
        if (it == asset.end()) {
            throw std::runtime_error("glTF asset object does not contain target glTF version");
        }

        if (!it.value().is_string()) {
            throw std::runtime_error("glTF asset has wrong the version value type");
        }

        auto version = it.value().get<std::string>();

        // it must contain major version only, rest part is optional
        std::regex re(u8"^(\\d+)($|.(\\d+))($|.\\d+$|(.\\d+.\\d+$))");
        std::smatch matches;

        if (std::regex_match(version, matches, re)) {
            assert(!matches.empty());

            auto majorVersion = static_cast<uint16_t>(strtol(matches[0].str().c_str(), nullptr, 10));

            return majorVersion >= minimalMajorVersion;
        }

        throw std::runtime_error("glTF asset has wrong the version format");
    }

    std::terminate();
}
}

#endif // CYCLONITE_GLTF_LOADER_INTERNAL_TESTVERSION_H
